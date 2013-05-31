<?php

/**
 * @file
 * Code related to send pattern to server / receive pattern as server.
 */


module_load_include('module', 'd2d');

function patterns_client_get_full_credentials($server, $user = NULL) {
	$conf = variable_get(PATTERNS_CLIENT_CREDENTIALS, FALSE);
	if (!$conf || empty($conf[$server])) return FALSE;
	
	if (!is_null($user) && $conf[$server]['user'] != $user) return FALSE;
	
	return $conf[$server];
	
}

function patterns_client_get_credentials($server, $user = NULL) {
	$conf = patterns_client_get_full_credentials($server, $user);
	if (!$conf) return FALSE;
	
	return array(
		'user' => $conf['user'],
		'pwd' => $conf['token'],
	);
}

function patterns_client_update_credentials($server, $user, $token, $email) {
	$current = variable_get(PATTERNS_CLIENT_CREDENTIALS, array());
	$current[$server] = array(
		'user' => $user,
		'token' => $token,
		'email' => $email,
	);
	variable_set(PATTERNS_CLIENT_CREDENTIALS, $current);
}


function patterns_client_is_user_registered_on_server($server, $user = NULL) {
  if (empty($server)) return FALSE;
  $credentials = patterns_client_get_credentials($server, $user);
  return $credentials ? TRUE : FALSE;  	
}

function patterns_client_register_user_on_server($url, $user, $pwd, $email) {
	
	$friend = d2d_api_friend_get_by_url($url);
	
	$post = array(
		'user' => $user,
		'pwd' => $pwd,
		'email' => $email,
	);
	
	// send the request
	$error_string = '';  
	$token = d2d_call_secure_rpc($friend, 'patterns_server_register_client', $post, $error_string);
	
	// Unknown Error
  if (is_null($token)) {
	  drupal_set_message(check_plain($url) . ' - ' . t('Unknown error'), 'error');
  }
  
	// Communication Error
	else if ($token === FALSE) {
	  drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
  }
  
  // Error while saving the user (e.g. already existing)
	else if (_patterns_client_is_registration_error($token)) {
    drupal_set_message(check_plain($url) . ' - ' . t('%err', array('%err' => $token)), 'error');
  }
  
  // save the results
  else {
  	drupal_set_message('Registration executed successfully. ' . check_plain($url)); // TODO: proper t()
  	patterns_client_update_credentials($url, $user, check_plain($token), $email);
    drupal_set_message('Credentials stored in local database'); // TODO: proper t()
  }
	
}


function patterns_client_link_user_on_server($url, $user, $pwd) {

	$friend = d2d_api_friend_get_by_url($url);
	$post = array(
		'user' => $user,
		'pwd' => $pwd,
  );
	
  if ($post === FALSE) {
    drupal_set_message(t('An error occurred while compacting your account data.'), 'error');
    return FALSE;
  }
  
	// send the request
	$error_string = '';  
	$result = d2d_call_secure_rpc($friend, 'patterns_server_link_client', $post, $error_string);
	
	// Unknown Error
  if (is_null($result)) {
	  drupal_set_message(check_plain($url) . ' - ' . t('Unknown error'), 'error');
  }
  
	// Communication Error
	else if ($result === FALSE) {
	  drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
  }
  
  // Trying to parse returned string
	else { 
	  $values = d2d_explode($result);
	  
	  if ($values === FALSE) {
	     drupal_set_message(check_plain($url) . ' - ' . t('Could not parse return value. Control server/client versions.'), 'error');
	  }
	  
	   $check = array(
    	'email' => 'is_string',
    	'pass' => 'is_string',
    );
	
    if (!d2d_check_array($values, $check)) {
       drupal_set_message(check_plain($url) . ' - ' . t('Could not extract the data from the encrypted message.'), 'error');
    }
    
  	// Another server error  
  	else if (_patterns_client_is_linking_error($values)) {
      drupal_set_message(check_plain($url) . ' - ' . t('%err', array('%err' => $token)), 'error');
    }
  
    // save the results
    else {
      $email = $values['email'];
      $token = $values['pass'];
  	  drupal_set_message('Existing username on server succesfully linked. ' . check_plain($url)); // TODO: proper t()
  	  patterns_client_update_credentials($url, $user, check_plain($token), check_plain($email));
      drupal_set_message('Credentials stored in local database'); // TODO: proper t()
    }
	}
	
}

function _patterns_client_is_registration_error($str) {
  $needle = 'Registration-Error.';
  return !strncmp($str, $needle, strlen($needle));
}

function _patterns_client_is_linking_error($str) {
  $needle = 'LinkingUser-Error.';
  return !strncmp($str, $needle, strlen($needle));
}


function patterns_client_push_patterns($patterns) {
  $url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
  
  if (!patterns_client_is_user_registered_on_server($url)) {
    drupal_set_message(t('No registered Patterns server found.'), 'error');
    return FALSE;
  }
  
  $credentials = patterns_client_get_credentials($url);
  
  $imploded_credentials = d2d_implode($credentials);
  
  if ($imploded_credentials === FALSE) {
     drupal_set_message(t('An error occurred while parsing your account data.'), 'error');
    return FALSE;
  }
  
  $upload_patterns = array();
  
  foreach ($patterns as $pattern) {
    $name = $pattern->name;
    $format = $pattern->format;

    $content = file_get_contents($pattern->file);
    if ($content === FALSE) {
      drupal_set_message(t('Pattern') . ' ' . check_plain($name) . ' ' . t('could not be processed'), 'error'); 
      continue;
    }
    $imploded_pattern = d2d_implode(
      array(
        'name' => $name,
        'format' => $format,
        'content' => $content,
      )
    );
    
    if ($imploded_pattern == FALSE) {
      drupal_set_message(t('An error occurred while processing pattern') . ' ' . check_plain($name) . ' ' . t('Skipped.'), 'error'); 
      continue;
    }
    
    $upload_patterns[] = $imploded_pattern;
  }

  $imploded_patterns = d2d_implode($upload_patterns);
  
  if ($imploded_patterns === FALSE) {
    drupal_set_message(t('Internal error while compacting the patterns to be sent.'), 'error'); 
    return FALSE;
  }
  
  
  $post = array(
    'credentials' => $imploded_credentials,
    'patterns' => $imploded_patterns,
  );
  
  

  // do we need this??
  //$imploded_post = d2d_implode($post);
  //
  //if ($imploded_post === FALSE) {
  //  drupal_set_message(t('Internal error while collecting patterns to be sent.'), 'error'); 
  //  return FALSE;
  //}
  ///
  
  $error_string = '';  
  
  $friend = d2d_api_friend_get_by_url($url);
  
  $res = d2d_call_secure_rpc($friend, 'patterns_server_publish_patterns', $post, $error_string);
  
  if ($res === FALSE) {
    drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
  }
  else {
    drupal_set_message('Upload to server successful.<br/>' . check_plain($url) . ' - ' . check_plain($res)); // TODO: proper t()
  }

}

/**
 * 
 * Performs a remote call to the Patterns Server using D2D in order
 * to retrieve the list of Patterns stored in the server sorted by its 
 * timestamp descendingly.
 * 
 * @param int $limit
 * @return array List of patterns currently stored in the server
 */
function _patterns_client_get_patterns_server_list($limit) {
  // Get the current server
  $url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
  $friend = d2d_api_friend_get_by_url($url);

  // Prepare arguments for the call. In this case just the maximum number of patterns expected
  $post = array(
      'limit' => $limit,
  );

  // Send the request. We explode the result in two different steps
  $error_string = '';
  $result = d2d_call_secure_rpc($friend, 'patterns_server_get_patterns_list', $post, $error_string);
  $result = d2d_explode($result);

  // Display communication errors if any
  if (!is_null($result['error'])) {
    drupal_set_message(t('The Patterns server returned this response: @response', array('@response' => $result['error'])), 'error');
  }else{
    //If everything is OK, return the results completely exploded
    $patterns_list = d2d_explode($result['return']);
    $exploded_result = array();
    foreach ($patterns_list as $p) {
      $exploded_result[] = d2d_explode($p);
    }
    return $exploded_result;
  }
}
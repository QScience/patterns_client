<?php

/**
 * @file
 * Code related to send pattern to server / receive pattern as server.
 */


module_load_include('module', 'pils');


function patterns_client_get_credentials($server, $user = NULL) {
	//return variable_get('patterns_client_user', FALSE);
	return array(
    'user' => 'client',
    'pwd' => '$S$DUXALZMQHtE3P08CvcaVhsOaacrQGr5JSTox19nPWQFIxrZxmGA0',
  );
}

function patterns_client_is_user_registered_on_server($url, $user = NULL) {
	return TRUE;
	
  if (empty($url)) return FALSE;
  $pc_user = variable_get('patterns_client_user', FALSE);
  
  return $pc_user && !empty($pc_user[$url]);  	
}


function patterns_client_get_friend($url) {
  $friends = drupaltodrupal_get_friends();
  foreach ($friends as $friend) {
    if ($friend['url'] === $url) {
      return $friend;
    }
  }
  return FALSE;
}

function patterns_client_register_user_on_server($url, $user, $pwd, $email) {
	
	$friend = patterns_client_get_friend($url);
	
	$post = pils_implode(array(
		'user' => $user,
		'pwd' => $pwd,
		'email' => $email,
	));
	
	$post = array(
		'user' => $user,
		'pwd' => $pwd,
		'email' => $email,
	);
	
	if ($post === FALSE) {
    drupal_set_message(t('An error occurred while compacting your account data.'), 'error');
    return FALSE;
  }
	
	// send the request
	$error_string = '';  
	$res = drupaltodrupal_call_secure_rpc($friend, 'patterns_server_register_client', $post, $error_string);
	
	// save the results
	if ($res === FALSE) {
    drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
  }
  else {
    drupal_set_message('Request sent successfully. ' . check_plain($url) . ' - ' .  check_plain($res)); // TODO: proper t()
  }
	
}

function patterns_client_push_patterns($patterns) {
  $url = variable_get('patterns_server', PATTERNS_CLIENT_SERVER);
  
  if (!patterns_client_is_user_registered_on_server($url)) {
    drupal_set_message(t('No registered Patterns server found.'), 'error');
    return FALSE;
  }
  
  $credentials = patterns_client_get_credentials($url);
  
  $imploded_credentials = pils_implode($credentials);
  
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
    $imploded_pattern = pils_implode(
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

  $imploded_patterns = pils_implode($upload_patterns);
  
  if ($imploded_patterns === FALSE) {
    drupal_set_message(t('Internal error while compacting the patterns to be sent.'), 'error'); 
    return FALSE;
  }
  
  
  $post = array(
    'credentials' => $imploded_credentials,
    'patterns' => $imploded_patterns,
  );
  
  

  // do we need this??
  //$imploded_post = pils_implode($post);
  //
  //if ($imploded_post === FALSE) {
  //  drupal_set_message(t('Internal error while collecting patterns to be sent.'), 'error'); 
  //  return FALSE;
  //}
  ///
  
  $error_string = '';  
  
  $friend = patterns_client_get_friend($url);
  
  $res = drupaltodrupal_call_secure_rpc($friend, 'patterns_server_publish_patterns', $post, $error_string);
  
  if ($res === FALSE) {
    drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
  }
  else {
    drupal_set_message('Upload to server successful.<br/>' . check_plain($url) . ' - ' . check_plain($res)); // TODO: proper t()
  }

}
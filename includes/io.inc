<?php

/**
 * @file
 * Code related to send pattern to server / receive pattern as server.
 */


module_load_include('module', 'pils');
// Pest
module_load_include('php', 'patterns_client', 'pest/Pest');

function patterns_client_registered_on_server($url) {
  if (empty($url)) return FALSE;
  
  // TODO ask the server if I am registered
  
  return TRUE;
}

function patterns_client_push_patterns($patterns) {
  $url = variable_get('patterns_server', PATTERNS_SERVER);
  
  if (!patterns_client_registered_on_server($url)) {
    drupal_set_message(t('No registered Patterns server found.'), 'error');
    return FALSE;
  }
  
  $credentials = array(
    'user' => 'client',
    'pwd' => '$S$Da.jp1uSIGwdcuqjMX8Nb6eX6A5rzjmpYGTnQxo.XJcHPckB3fZC',
  );
  
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
  
  

  $imploded_post = pils_implode($post);
  
  if ($imploded_post === FALSE) {
    drupal_set_message(t('Internal error while collecting patterns to be sent.'), 'error'); 
    return FALSE;
  }
  
   // Create pest object. Will send them all in one connection
  $pest = new Pest($url);
  
  try {
    $results = $pest->post('/post', $post);
    drupal_set_message('Communication DONE: ' . $results);
//      $error_string = '';
  //  if ($results === FALSE) {
//    drupal_set_message(check_plain($url) . ' - ' . check_plain($error_string), 'error');
//  }
//  else {
//    drupal_set_message('Upload to server successful.<br/>' . check_plain($url) . ' - ' . check_plain($res)); // TODO: proper t()
//  }
    
  }
  catch(Exception $e) {
    drupal_set_message('patterns_client: A communication error occurred.', 'error');
    drupal_set_message($e->getMessage(), 'error');
  }
  
  
  
  

  
  
}
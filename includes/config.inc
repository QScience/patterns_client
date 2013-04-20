<?php

/**
 * @file
 * Forms and settings for Patterns Client
 */


//function patterns_client_d2d_friends_select($form = array(), $name = 'd2dfriends') {
//  $friends = d2d_get_friends();
//}

function patterns_client_form_outgoing_configure() {
  $form = array();
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('Address of a central server to which patterns should be published'),
    '#default_value' => variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER),
    '#size' => 40,
    '#maxlength' => PATTERNS_CLIENT_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['auto'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('patterns_client_auto_publish', FALSE),
    '#title' => t('Automatically publish patterns'),
    '#description' => t('If checked, patterns are automatically published whenever is marked as \'public\'.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

function patterns_client_form_incoming_configure_submit($form, &$form_state) {
  variable_set('patterns_client_allow_publish', $form_state['values']['allow'] ? TRUE : FALSE);
  drupal_set_message(t('The changes have been saved.'));
}

function patterns_client_form_outgoing_configure_submit($form, &$form_state) {
  variable_set('patterns_client_auto_publish', $form_state['values']['auto'] ? TRUE : FALSE);
  variable_set('patterns_client_server', $form_state['values']['address']);
  drupal_set_message(t('The changes have been saved.'));
}

function patterns_client_config() {
  $build['main'] = array(
    '#title' => t('Patterns Server'),
    '#type' => 'fieldset',
  );
  $build['main'][] = drupal_get_form('patterns_client_form_outgoing_configure');
  if (variable_get('patterns_client_auto_publish', FALSE)) {
    $url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
    
    $credentials = patterns_client_get_full_credentials($url);
    
    if (!$credentials) {
    	$build['register'] = array(
    		'#title' => t('Register a new username'),
    		'#type' => 'fieldset',
  		); 
    	$build['register'][] = array(
    		'#markup' => '<strong>You are currently not registered on the server</strong>',
    	);
    	
    	$build['register'][] = drupal_get_form('patterns_client_register_user');
      drupal_set_message(t('You are not registered on server @url.', array('@url' => $url)), 'error');
    }
    else {
      drupal_set_message(t('You are correctly registered on server @url.', array('@url' => $url)));
      
      $build['credentials'] = array(
      	'#title' => 'Your credentials',
      	'#type' => 'fieldset',
      );
      
      $info['Server'][] = t('Server');
  	  $info['Server'][] = $url;
  	  $info['User'][] = t('User');
      $info['User'][] = $credentials['user'];
      $info['Token'][] = t('Token');
      $info['Token'][] = $credentials['token'];
      $info['Email'][] = t('Email');
      $info['Email'][] = $credentials['email'];
    
      $build['credentials'][] = array(
    		'#markup' => theme('table', array('rows' => $info, 'attributes' => array('class' => 'patterns-list'))),
    	);
      
      
    }
  }
  return $build;
}

function patterns_client_register_user() {
  $form = array();
  $form['user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Username to register on the server'),
    //'#default_value' => variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER),
    '#size' => 40,
    '#maxlength' => PATTERNS_CLIENT_USER_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
  	'#required' => TRUE,
  );
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
  	'#required' => TRUE,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Register'),
  );
  
  return $form;
}

function patterns_client_register_user_validate($form, &$form_state) {
	if (!variable_get('patterns_client_server', FALSE)) {
		drupal_set_message(t('No server found to register a new user'), 'error');
		return FALSE;
	}
	
	if (!valid_email_address($form_state['values']['email'])) {
  	form_set_error('email', t('The email address appears to be invalid.'));
	}
	
	return TRUE;
}

function patterns_client_register_user_submit($form, &$form_state) {
	$url = variable_get('patterns_client_server', PATTERNS_CLIENT_SERVER);
	$user = $form_state['values']['user'];
	$pwd = $form_state['values']['password'];
	$email = $form_state['values']['email'];
	patterns_client_register_user_on_server($url, $user, $pwd, $email);
}
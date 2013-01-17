<?php

/**
 * @file
 * Forms and settings for Patterns Client
 */

function patterns_client_form_outgoing_configure() {
  $form = array();
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#description' => t('Address of a central server to which patterns should be published'),
    '#default_value' => variable_get('patterns_server', PATTERNS_SERVER),
    '#size' => 40,
    '#maxlength' => PATTERNS_CLIENT_URL_MAX_LENGTH,
    '#required' => TRUE,
  );
  $form['auto'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('patterns_d2d_auto_publish', FALSE),
    '#title' => t('Automatically publish patterns'),
    '#description' => t('If checked, patterns are automatically published whenever the \'publish\'-button is hit.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}
function patterns_client_form_incoming_configure() {
  $form = array();
//  $form['allow'] = array(
//    '#type' => 'checkbox',
//    '#default_value' => variable_get('patterns_client_allow_publish', FALSE),
//    '#title' => t('Allow patterns to be published'),
//    '#description' => t('If checked, friends with the required permissions can publish patterns on this server, see D2D\'s groups/permissions.'),
//  );
//  $form['submit'] = array(
//    '#type' => 'submit',
//    '#value' => t('Save'),
//  );
  return $form;
}
function patterns_client_form_incoming_configure_submit($form, &$form_state) {
  variable_set('patterns_client_allow_publish', $form_state['values']['allow'] ? TRUE : FALSE);
  drupal_set_message(t('The changes have been saved.'));
}
function patterns_client_form_outgoing_configure_validate($form, &$form_state) {
  if (!drupaltodrupal_check_url($form_state['values']['address'])) {
    form_set_error('address', t('Address must start with \'http://\' or \'https://\' and end with \'/\'.'));
  }
}
function patterns_client_form_outgoing_configure_submit($form, &$form_state) {
  if ($form_state['values']['auto']) {
    drupaltodrupal_blindly_add_friend($form_state['values']['address'], 'patterns server', FALSE);
  }
  variable_set('patterns_client_auto_publish', $form_state['values']['auto'] ? TRUE : FALSE);
  variable_set('patterns_server', $form_state['values']['address']);
  drupal_set_message(t('The changes have been saved.'));
}
function patterns_client_config() {
  $build['main'] = array(
    '#title' => t('Sharing settings'),
    '#type' => 'fieldset',
  );
  $build['main'][] = drupal_get_form('patterns_client_form_outgoing_configure');
  $build['main'][] = drupal_get_form('patterns_client_form_incoming_configure');
  if (variable_get('patterns_client_auto_publish', FALSE)) {
    $url = variable_get('patterns_server', PATTERNS_SERVER);
    if (patterns_client_registered_on_server($url)) {
      drupal_set_message(t('You are not registered on server @url.', array('@url' => $url)), 'error');
    }
    else {
      drupal_set_message(t('You are correctly registered on server @url.', array('@url' => $url)));
    }
  }
  return $build;
}

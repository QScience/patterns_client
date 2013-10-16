<?php

/**
 * @file
 * Provides utils for the own module or for developers.
 */

/**
 * Display information regarding current
 *
 * Uses _drupal_set_message_ to display links to the configuration page.
 *
 * @return bool|array
 *   Associative array with url of server and credentials, or FALSE if they
 *   are not set correctly.
 */
function patterns_client_check_credentials_and_display_warns() {

  $out = array(
    'url' => FALSE,
    'credentials' => FALSE,
    'friend' => FALSE,
    'ready' => FALSE,
  );

  // Get the current server.
  $url = variable_get('patterns_client_server');

  if (empty($url)) {
    drupal_set_message(t('You are not registered to any Patterns server.'), 'warning');
    drupal_set_message(t('Check the Patterns Client !configuration.', array(
      '!configuration' => l(t('configuration'), 'admin/patterns/share'),
    )));
    return $out;
  }
  else {
    $out['url'] = $url;
    $friend = d2d_api_friend_get_by_url($url);
    if (empty($friend)) {
      $pserver_link = l(t('Patterns server'), $url, array(
        'attributes' => array('target'=>'_blank'),
      ));
      drupal_set_message(t('You are currently not friend with !server.',
          array( '!server' => $pserver_link )), 'warning');
      drupal_set_message(t('Check the Patterns Client !configuration.', array(
        '!configuration' => l(t('configuration'), 'admin/patterns/share'),
      )));
    }
    else {
      $out['friend'] = TRUE;

      $credentials = patterns_client_get_full_credentials($url);

      if (empty($credentials)) {
        $pserver_link = l(t('Patterns server'), $url, array(
          'attributes' => array('target'=>'_blank'),
        ));
        drupal_set_message(t('You have not registered a username on the !server yet.',
            array( '!server' => $pserver_link )), 'warning');
        drupal_set_message(t('Check the Patterns Client !configuration.', array(
          '!configuration' => l(t('configuration'), 'admin/patterns/share'),
        )));
      }
      else {
        $out['credentials'] = $credentials;
        $out['ready'] = TRUE;
      }
    }
  }
  return $out;
}
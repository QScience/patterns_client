<?php 

/**
 * Submit handler to query xmlrpc_example.add.
 *
 * Submit: query the XML-RPC endpoint for the method xmlrpc_example.add
 * and report the result as a Drupal message.
 *
 * @param $form
 *   form array
 * @param $form_state
 *   form_state array
 *
 * @see xmlrpc()
 * @see xmlrpc_errno()
 * @see xmlrpc_error_msg()
 */
function _patterns_client_get_patterns_list() {
  // First define the endpoint of the XML-RPC service, in this case is our
  // own server.
  $server = url('http://localhost/qsi_server' . '/xmlrpc.php', array('external' => TRUE));
  // Then we should define the method to call. xmlrpc() requires that all the
  // information related to the called method is passed as an array in the form
  // of 'method_name' => arguments_array
  $options = array(
      'patterns_server.get_patterns_list' => array(
          10,
      ),
  );
  // Make the xmlrpc request and process the results.
  $result = xmlrpc($server, $options);
  if ($result === FALSE) {
    drupal_set_message(
    t('Error return from xmlrpc(): Error: @errno, Message: @message',
    array('@errno' => xmlrpc_errno(), '@message' => xmlrpc_error_msg())),
    'error'
        );
  }
  else {
    echo(
        t('The XML-RPC server returned this response: @response',
            array('@response' => print_r($result, TRUE)))
    );
  }
}
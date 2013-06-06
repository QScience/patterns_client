<?php

/**
 * 
 * Checks the existance of previous parents and prepends the
 * given one at the beginning.
 * 
 * @param array $pattern  In an array format, as received from the parser
 * @param string $uuuid  Parent UUUID
 * @return array Pattern with the new parent prepended
 */
function _patterns_client_add_parent($pattern, $uuuid) {
  if (isset($pattern['info']['parents'])) {
    array_unshift($pattern['info']['parents'], $uuuid);
  }else{
    $pattern['info']['parents'][] = $uuuid;
  }
  return $pattern;
}
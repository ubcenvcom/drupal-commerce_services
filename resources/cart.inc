<?php

/**
 * @file
 * Contains the resource callbacks for cart orders.
 */


/**
 * Returns a collection of cart orders for the current user.
 *
 * @see commerce_services_index_entities()
 */
function commerce_services_cart_index($fields, $expand_entities, $flatten_fields, $filter, $filter_op, $sort_by, $sort_order, $limit, $offset) {
  global $user;

  // For now, this resource only returns the current cart order of the given API
  // user. We can expand the collection to include all cart orders sorted so the
  // current order is first in the list at a later time.
  $uid = !empty($user->uid) ? $user->uid : 0;
  $order_id = commerce_cart_order_id($uid);

  if (!empty($order_id)) {
    $filter['order_id'] = $order_id;
    $filter_op['order_id'] = '=';
  }
  else {
    return array();
  }

  return commerce_services_index_entities('commerce_order', $fields, $expand_entities, $flatten_fields, $filter, $filter_op, $sort_by, $sort_order, $limit, $offset);
}

/**
 * Determines an API user's access to index cart orders.
 */
function commerce_services_cart_index_access() {
  // Because the entity access control system will filter any result sets,
  // there's no reason to limit a particular user's access.
  return TRUE;
}

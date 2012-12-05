<?php

/**
 * @file
 * Contains the resource callbacks for product display nodes.
 */


/**
 * Returns a collection of product display nodes.
 *
 * @param $fields
 *   An array of field and property names to include in the returned product
 *   display node data. If empty, nodes will be returned as loaded with the
 *   exception of rdf_mapping data, which will only ever be included in an index
 *   if it is explicitly requested.
 * @param $limit
 *   The number of product displays to retrieve.
 * @param $offset
 *   The zero-based index from which to start retrieving product displays.
 *
 * @return
 *   An array of product display nodes.
 */
function commerce_services_product_display_index($fields, $limit, $offset) {
  // Build a query to load all accessible nodes that contain product reference
  // fields, whether those fields have data or not.
  $query = new EntityFieldQuery();
  $query
    ->addTag('node_access')
    ->addMetaData('op', 'view')
    ->addMetaData('base_table', 'node')
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', array_keys(commerce_product_reference_node_types()))
    ->propertyCondition('status', 1)
    ->propertyOrderBy('sticky', 'DESC')
    ->propertyOrderBy('created', 'DESC')
    ->range($offset, $limit);

  $result = $query->execute();

  // Load the nodes returned by the query.
  $nodes = node_load_multiple(array_keys($result['node']));

  // Unset the rdf_mapping key from any loaded node unless explicitly included
  // in the $fields array.
  if (!in_array('rdf_mapping', $fields)) {
    foreach ($nodes as $nid => $node) {
      unset($node->rdf_mapping);
    }
  }

  // Filter out the node properties based on the field list if given; always
  // remove rdf_mapping unless explicitly requested in the field list.
  if (!empty($fields)) {
    foreach ($nodes as $nid => &$node) {
      commerce_services_object_filter($node, $fields);
    }
  }

  return $nodes;
}

/**
 * Determines access to perform product display operations.
 *
 * @param $op
 *   The operation being performed. One of 'view', 'update', 'create' or
 *   'delete'.
 * @param $args
 *   Resource arguments passed through from the original request.
 *
 * @return
 *   Boolean indicating whether or not the requested operation should be allowed.
 */
function commerce_services_product_display_access($op, $args) {
  // First load the node ID from the arguments if available.
  if ($op != 'create' && !empty($args[0])) {
    $node = node_load($args[0]);
  }

  // If the request operation is a node create...
  if ($op == 'create') {
    // And the POSTed node object has a valid product display node type...
    if (!empty($args[0]->type) && in_array($args[0]->type, commerce_product_reference_node_types())) {
      // Return the user's access to create nodes of that type.
      if (!node_access($op, $args[0]->type)) {
        return services_error(t('Access to this operation not granted'), 401);
      }
      else {
        return TRUE;
      }
    }
    else {
      // Otherwise throw an error with an appropriate error message.
      return services_error(t('Valid product display node type required'), 400);
    }
  }

  // If the node could be loaded, return the node_access() value for the operation.
  if (!empty($node->nid)) {
    if (!node_access($op, $node)) {
      return services_error(t('Access to this operation not granted'), 401);
    }
    else {
      return TRUE;
    }
  }
  else {
    return services_error(t('Product display not found'), 404);
  }
}

/**
 * Returns a single product display node.
 *
 * @param $nid
 *   The node ID of the product display to retrieve.
 *
 * @return
 *   The requested product display node or FALSE if not found.
 */
function commerce_services_product_display_retrieve($nid) {
  $node = node_load($nid);

  // If the node exists, determine its URI and add it to the node.
  if ($node) {
    $uri = entity_uri('node', $node);
    $node->path = url($uri['path'], array('absolute' => TRUE));

    // Unset the uri property if it's on the node to prevent recursion.
    unset($node->uri);
  }

  return $node;
}
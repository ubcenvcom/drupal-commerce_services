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
 * Returns a single product display node.
 *
 * @param $nid
 *   The node ID of the product display to retrieve.
 *
 * @return
 *   The requested product display node or FALSE if not found.
 */
function commerce_services_product_display_retrieve($nid) {
  return node_load($nid);
}

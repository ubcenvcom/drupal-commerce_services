<?php

/**
 * @file
 * Contains the resource callbacks for products.
 */


/**
 * Returns a collection of products.
 *
 * @param $fields
 *   An array of field and property names to include in the returned product
 *   data. If empty, products will be returned as loaded.
 * @param $expand_entities
 *   An integer depth to which entity reference field values should be expanded
 *   into full entities in a corresponding property on returned entities;
 *   defaults to 1.
 * @param $flatten_fields
 *   A string indicating whether or not field value arrays should be flattened;
 *   accepts true or false, defaults to true.
 * @param $filter
 *   An associative array of property names, single column field names, or
 *   multi-column field column names with their values to use to filter the
 *   result set of the index request.
 * @param $filter_op
 *   An associative array of field and property names with the operators to use
 *   when applying their filter conditions to the index request query.
 * @param $sort_by
 *   An array of database fields to sort the query by, with sort fields being
 *   valid properties, single column field names, or multi-column field column
 *   names for the matching entity type.
 * @param $sort_order
 *   The corresponding sort orders for the fields specified in the $sort_by
 *   array; one of either 'DESC' or 'ASC'.
 * @param $limit
 *   The number of products to retrieve.
 * @param $offset
 *   The zero-based index from which to start retrieving products.
 *
 * @return
 *   An array of products.
 */
function commerce_services_product_index($fields, $expand_entities, $flatten_fields, $filter, $filter_op, $sort_by, $sort_order, $limit, $offset) {
  // Build a query to load all accessible products.
  $query = new EntityFieldQuery();
  $query
    ->addTag('commerce_product_access')
    ->entityCondition('entity_type', 'commerce_product')
    ->range($offset, $limit);

  // Add the specified property and field condtions to the query.
  commerce_services_index_query_filter($query, 'commerce_product', $filter, $filter_op);

  // Add the specified sort conditions to the query.
  commerce_services_index_query_sort($query, 'commerce_product', $sort_by, $sort_order);

  // Execute the query and load the products it returns if any.
  $result = $query->execute();
  $products = array();

  if (!empty($result['commerce_product'])) {
    $products = commerce_product_load_multiple(array_keys($result['commerce_product']));

    foreach ($products as $product_id => &$product) {
      // Add simplified fields to the entity object for certain field types.
      commerce_services_decorate_entity('commerce_product', $product);

      // Expand referenced entities into the full entity to the specified depth.
      if ($expand_entities > 0) {
        commerce_services_expand_entities('commerce_product', $product, $expand_entities, $flatten_fields == 'true');
      }

      // Flatten field value arrays if specified. This must be the last operation
      // performed as it breaks the standard field data model. An entity whose
      // fields have thus been flattened is no longer wrappable.
      if ($flatten_fields == 'true') {
        $product = clone($product);
        commerce_services_flatten_fields('commerce_product', $product);
      }

      // Filter out the product properties based on the field list if given.
      if (!empty($fields)) {
        commerce_services_object_filter($product, $fields);
      }
    }
  }

  return $products;
}

/**
 * Determines an API user's access to index products.
 */
function commerce_services_product_index_access() {
  // Because the entity access control system will filter any result sets,
  // there's no reason to limit a particular user's access.
  return TRUE;
}

/**
 * Returns a single product.
 *
 * @param $product_id
 *   The product ID of the product to retrieve.
 * @param $expand_entities
 *   An integer depth to which entity reference field values should be expanded
 *   into full entities in a corresponding property on returned entities;
 *   defaults to 1.
 * @param $flatten_fields
 *   A string indicating whether or not field value arrays should be flattened;
 *   accepts true or false, defaults to true.
 *
 * @return
 *   The requested product or FALSE if not found.
 */
function commerce_services_product_retrieve($product_id, $expand_entities, $flatten_fields) {
  $product = commerce_product_load($product_id);

  if ($product) {
    // Add simplified fields to the entity object for certain field types.
    commerce_services_decorate_entity('commerce_product', $product);

    // Expand referenced entities into the full entity to the specified depth.
    if ($expand_entities > 0) {
      commerce_services_expand_entities('commerce_product', $product, $expand_entities, $flatten_fields == 'true');
    }

    // Flatten field value arrays if specified. This must be the last operation
    // performed as it breaks the standard field data model. An entity whose
    // fields have thus been flattened is no longer wrappable.
    if ($flatten_fields == 'true') {
      $product = clone($product);
      commerce_services_flatten_fields('commerce_product', $product);
    }
  }

  return $product;
}

/**
 * Determines an API user's access to retrieve a given product.
 *
 * @param $product_id
 *   The ID of the product to be retrieved.
 *
 * @return
 *   Boolean indicating the user's access to retrieve the product.
 */
function commerce_services_product_retrieve_access($product_id) {
  // Attempt to load the product.
  if ($product = commerce_product_load($product_id)) {
    // And perform the view access check.
    if (commerce_product_access('view', $product)) {
      return TRUE;
    }
    else {
      return services_error(t('Access to this operation not granted'), 401);
    }
  }
  else {
    return services_error(t('Product not found'), 404);
  }
}

/**
 * Deletes a product.
 *
 * @param $product_id
 *   The ID of the product to be deleted.
 */
function commerce_services_product_delete($product_id) {
  commerce_product_delete($product_id);
}

/**
 * Determines an API user's access to delete a given product.
 *
 * @param $product_id
 *   The ID of the product to be deleted.
 *
 * @return
 *   Boolean indicating the user's access to delete the product.
 */
function commerce_services_product_delete_access($product_id) {
  // Attempt to load the product.
  if ($product = commerce_product_load($product_id)) {
    // If the user has access to perform the operation...
    if (commerce_product_access('delete', $product)) {
      // And the product can safely be deleted...
      if (commerce_product_can_delete($product)) {
        return TRUE;
      }
      else {
        return services_error(t('Product cannot be deleted'), 403);
      }
    }
    else {
      return services_error(t('Access to this operation not granted'), 401);
    }
  }
  else {
    return services_error(t('Product not found'), 404);
  }
}

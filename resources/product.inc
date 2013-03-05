<?php

/**
 * @file
 * Contains the resource callbacks for products.
 */


/**
 * Returns a collection of products.
 *
 * @see commerce_services_index_entities()
 */
function commerce_services_product_index($fields, $expand_entities, $flatten_fields, $filter, $filter_op, $sort_by, $sort_order, $limit, $offset) {
  return commerce_services_index_entities('commerce_product', $fields, $expand_entities, $flatten_fields, $filter, $filter_op, $sort_by, $sort_order, $limit, $offset);
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
 * @see commerce_services_retrieve_entity()
 */
function commerce_services_product_retrieve($product_id, $expand_entities, $flatten_fields) {
  return commerce_services_retrieve_entity('commerce_product', $product_id, $expand_entities, $flatten_fields);
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
 * Updates a product.
 *
 * @see commerce_services_update_entity()
 */
function commerce_services_product_update($product_id, $data, $flatten_fields) {
  return commerce_services_update_entity('commerce_product', $product_id, $data, $flatten_fields);
}

/**
 * Determines an API user's access to update a given product.
 *
 * @param $product_id
 *   The ID of the product to be updated.
 *
 * @return
 *   Boolean indicating the user's access to update the product.
 */
function commerce_services_product_update_access($product_id) {
  // Attempt to load the product.
  if ($product = commerce_product_load($product_id)) {
    // If the user has access to perform the operation...
    if (commerce_product_access('update', $product)) {
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

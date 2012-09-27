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
 * @param $limit
 *   The number of products to retrieve.
 * @param $offset
 *   The zero-based index from which to start retrieving products.
 *
 * @return
 *   An array of products.
 */
function commerce_services_product_index($fields, $limit, $offset) {
  // Build a query to load all accessible products.
  $query = new EntityFieldQuery();
  $query
    ->addTag('commerce_product_access')
    ->entityCondition('entity_type', 'commerce_product')
    ->propertyOrderBy('product_id')
    ->range($offset, $limit);

  $result = $query->execute();

  // Load the products returned by the query.
  $products = commerce_product_load_multiple(array_keys($result['commerce_product']));

  // Filter out the product properties based on the field list if given.
  if (!empty($fields)) {
    foreach ($products as $product_id => &$product) {
      commerce_services_object_filter($product, $fields);
    }
  }

  return $products;
}

/**
 * Returns a single product.
 *
 * @param $product_id
 *   The product ID of the product to retrieve.
 *
 * @return
 *   The requested product or FALSE if not found.
 */
function commerce_services_product_retrieve($product_id) {
  return commerce_product_load($product_id);
}

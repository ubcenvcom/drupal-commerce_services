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
 * Updates a product.
 *
 * @param $product_id
 *   The ID of the product to be updated.
 * @param $data
 *   The product data to update.
 * @param $flatten_fields
 *   A string indicating whether or not field value arrays in the data are
 *   flattened as in GET requests; accepts true or false, defaults to true.
 */
function commerce_services_product_update($product_id, $data, $flatten_fields) {
  watchdog('debug', $product_id . ' - <pre>'. print_r($data, TRUE) .'</pre>');

  $product = commerce_product_load($product_id);
  $product_wrapper = entity_metadata_wrapper('commerce_product', $product);
  $properties = entity_get_property_info('commerce_product');
  $bundle = field_extract_bundle('commerce_product', $product);

  // Build an array of restricted properties, preventing update requests from
  // changing IDs and other immutable properties of the entity.
  $entity_info = entity_get_info('commerce_product');
  $entity_keys = array_diff_key($entity_info['entity keys'], array('label' => ''));

  $restricted_properties = array_values($entity_keys);

  // Loop over the data submitted with the request.
  foreach ($data as $key => $value) {
    // If the current key is a restricted property, return the error.
    if (in_array($key, $restricted_properties)) {
      return services_error(t('Property @property cannot be changed', array('@property' => $key)), 400);
    }

    // If the key is a property name...
    if (in_array($key, commerce_services_entity_type_properties('commerce_product'))) {
      // If valid, set it directly to the product.
      if (commerce_services_validate_property_value('commerce_product', $product, $key, $value)) {
        $product->{$key} = $value;
      }
      else {
        return services_error(t('Invalid @property value given', array('@property' => $key)), 400);
      }
    }
    else {
      // Look for the key as a field name in the entity type's field list.
      $known_field = FALSE;

      foreach (commerce_services_entity_type_fields('commerce_product') as $field_name => $field_type) {
        // If the key begins with a field name, then either the key is the field
        // name or is a column of the field.
        if (strpos($key, $field_name) === 0) {
          $field_info = field_info_field($field_name);

          // Because the entity metadata wrapper treats text fields differently
          // based on whether or not they enable format selection, we also
          // accommodate the inconsistency here. If a text field does not have
          // text formatting enabled, we unset that column from the field schema
          // so it can be set independently as if it were the only column in the
          // field value.
          if (in_array($field_info['type'], array('text', 'text_long'))) {
            $instance_info = field_info_instance('commerce_product', $field_name, $product->type);

            if (empty($instance_info['settings']['text_processing'])) {
              unset($field_info['columns']['format']);
            }
          }

          if ($field_name == $key) {
            // If the update request sent fields in a flattened format...
            if ($flatten_fields == 'true') {
              // Update the field value without specifying a language code.
              if ($field_info['cardinality'] == 1) {
                commerce_services_update_field_value('commerce_product', $product, $field_info, $value);
              }
              else {
                // For multi-value fields, loop over the value array and pass
                // the delta value to the update function.
                foreach ($value as $delta => $item) {
                  commerce_services_update_field_value('commerce_product', $product, $field_info, $item, NULL, $delta);
                }
              }
            }
            else {
              // Otherwise, loop over the languages given and attempt to update
              // the field value individually for each language code. Since
              // field values are not flattened, the value array must include a
              // language code and delta values even for single-value fields.
              foreach ($value as $langcode => $items) {
                // Return an error if the $langcode is unknown or if $items is
                // not an array.
                if ($langcode != LANGUAGE_NONE && !in_array($langcode, array_keys(language_list()))) {
                  return services_error(t('Unknown language @langcode in @field value', array('@langcode' => $langcode, '@field' => $field_name)), 400);
                }
                elseif (!is_array($items)) {
                  return services_error(t('Invalid @field value given', array('@field' => $field_name)), 400);
                }

                foreach ($items as $delta => $item) {
                  // If $item is empty, simply unset it at the delta value.
                  if (empty($item)) {
                    unset($product->{$field_name}[$langcode][$delta]);
                  }
                  else {
                    // Otherwise update the field value for the delta value.
                    commerce_services_update_field_value('commerce_product', $product, $field_info, $item, $langcode, $delta);
                  }
                }
              }
            }

            $known_field = TRUE;
            break;
          }
          else {
            // Otherwise if the key contains a valid column specification for
            // the field type, create a value array to set the column directly.
            $column = substr($key, strlen($field_name) + 1);

            if (in_array($column, array_keys($field_info['columns']))) {
              if ($flatten_fields == 'true') {
                // Directly set single value fields.
                if ($field_info['cardinality'] == 1) {
                  commerce_services_update_field_value('commerce_product', $product, $field_info, array($column => $value));
                }
                else {
                  // For multi-value fields, loop over the value array and pass
                  // the delta value to the update function.
                  foreach ($value as $delta => $item) {
                    commerce_services_update_field_value('commerce_product', $product, $field_info, array($column => $item), NULL, $delta);
                  }
                }
              }
              else {
                // If field values are not flattened, they must include both a
                // language code and delta values even for single-value fields.
                foreach ($value as $langcode => $items) {
                  foreach ($items as $delta => $item) {
                    commerce_services_update_field_value('commerce_product', $product, $field_info, array($column => $item), $langcode, $delta);
                  }
                }
              }

              $known_field = TRUE;
              break;
            }
          }
        }
      }

      // If no known field matched, return a 400 error.
      if (!$known_field) {
        return services_error(t('Unknown property @name', array('@name' => $key)), 400);
      }
    }
  }

  // Save the updates to the product.
  commerce_product_save($product);

  // @todo Post-process textfields by invoking text_field_load() on their values
  // to get the updated summary and safe_value values.

  // Add simplified fields to the entity object for certain field types.
  commerce_services_decorate_entity('commerce_product', $product);

  // Flatten field value arrays if specified. This must be the last operation
  // performed as it breaks the standard field data model. An entity whose
  // fields have thus been flattened is no longer wrappable.
  if ($flatten_fields == 'true') {
    $product = clone($product);
    commerce_services_flatten_fields('commerce_product', $product);
  }

  return $product;
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

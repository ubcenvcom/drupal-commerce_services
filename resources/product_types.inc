<?php

/**
 * @file
 * Contains the resource callbacks for product types.
 */

function commerce_services_product_types_index_access() {
 return TRUE;
}

function commerce_services_product_types_index() {
 $r=array();
 $pts=commerce_product_types();

 foreach ($pts as $id => $info) {
   $r[$id]=$info;
   $field_instances = field_info_instances('commerce_product', $id);
   foreach ($field_instances as $field_name => $instance) {
    $field_info = field_info_field($field_name);

    // Collect common details from the information overload given
    $data=array();
    $data['type']=$field_info['type'];

    $r[$id]['fields'][$field_name]=$data;
   }
 }
 return $r;
}

<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: product_management.inc,v 1.9.6.1 2008/04/18 06:54:41 bshkara Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Product_Management
*
* @see Asset_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.9.6.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Product_Management extends Asset_Management
{

	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	the package manager to use
	*
	* @return void
	* @access public
	*/
	function Product_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// include price class so unserialize() doesn't barf
		require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';

		// images for products will be linked assets so dont include here
		// Also quantity is not an issue at the moment
		// NB: Price is 0 (zero) if free, or -1 if unknown
		$this->vars = array(
						'name' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'The name of the Product that this asset represents',
						),
						'price' => array(
							'added'			=> '0.1',
							'type'			=> 'float',
							'default'		=> 0.0,
							'description'	=> 'Sale value of this Product',
							'is_admin'		=> false,
						),
						'price_obj' => array(
							'added'			=> '0.1',
							'type'			=> 'serialise',
							'default'		=> new Product_Price(),
							'description'	=> 'Proper object for manipulation of the price',
						),
			/*
						'quantity' => array(
							'added'			=> '0.1',
							'type'			=> 'int',
							'default'		=> 0,
							'description'	=> 'Quantity of the item on hand',
						),
			*/
						'description' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Long description of the product',
							'is_admin'		=> false,
						),
						'short_description' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Short description of product for small display space',
							'is_admin'		=> false,
						),
						'details' => array(
							'added'			=> '0.1',
							'type'			=> 'text',
							'default'		=> '',
							'description'	=> 'Detailed specifications of the Product',
							'is_admin'		=> false,
						),
		);

	}//end Product_Management()


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset should be upgraded to
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version,'0.2','<')) {
			// renames an instance variable of the Price object included in all Product objects
			// 'tax_inclusive' to 'value_has_tax'
			// also will set 'calculate_tax' if 'tax_inclusive' was previously set

			pre_echo('UPGRADING PRODUCT ASSET - FROM VERSION '.$current_version.' TO VERSION 0.2');

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			// get attribute id of the price object
			$sql = 'SELECT DISTINCT sq_ast_attr_val.attrid
					FROM sq_ast_attr_val
					JOIN sq_ast_attr
					ON sq_ast_attr_val.attrid = sq_ast_attr.attrid
					WHERE sq_ast_attr.type_code = '.$db->quote('product').'
					AND sq_ast_attr.name = '.$db->quote('price_obj');

			$attrid = $db->getOne($sql);
			assert_valid_db_result($attrid);

			// get all assetids and their custom_vals
			$sql = 'SELECT assetid, custom_val
					FROM sq_ast_attr_val
					WHERE attrid = '.$db->quote($attrid);

			$custom_vals = $db->getAssoc($sql);
			assert_valid_db_result($custom_vals);

			pre_echo('UPGRADING THE PRICE_OBJ ATTRIBUTE WITH ATTRID: '.$attrid);

			// include the price class so unserialize() doesn't barf
			require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';
			// loop through all custom_vals and perform our updates
			foreach ($custom_vals as $assetid => $custom_val) {
				$object = unserialize($custom_val);
				if (isset($object->tax_inclusive)) {
					$object->value_has_tax = $object->tax_inclusive;
					unset($object->tax_inclusive);
					// if tax_inclusive was previously set, they would have wanted tax calculated
					if ($object->value_has_tax) {
						$object->calculate_tax = TRUE;
					}
				}
				$serialized = serialize($object);

				// now we can update our db with this new value
				$sql = 'UPDATE sq_ast_attr_val
						SET custom_val = '.$db->quote($serialized).'
						WHERE attrid = '.$db->quote($attrid).'
						AND assetid = '.$db->quote($assetid);

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}

			// lastly change the default value
			// first get the default value
			$sql = 'SELECT default_val
					FROM sq_ast_attr
					WHERE attrid = '.$db->quote($attrid);

			$default_val = $db->getOne($sql);
			assert_valid_db_result($default_val);

			// alter it
			$object = unserialize($default_val);
			if (isset($object->tax_inclusive)) {
				$object->value_has_tax = $object->tax_inclusive;
				unset($object->tax_inclusive);
			}
			$serialized = serialize($object);

			$sql = 'UPDATE sq_ast_attr
					SET default_val = '.$db->quote($serialized).'
					WHERE attrid = '.$db->quote($attrid);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			pre_echo('UPGRADE FOR PRODUCT ASSETS COMPLETE');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>

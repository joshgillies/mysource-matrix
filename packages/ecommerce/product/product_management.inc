<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: product_management.inc,v 1.15 2008/11/25 00:48:06 mbrydon Exp $
*
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Product_Management
*
* @see Asset_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.15 $
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
			$db = MatrixDAL::getDb();

			// get all type descendants
			$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('product', TRUE);
			for ($i=0; $i<count($type_codes); $i++) {
				$type_codes[$i] = MatrixDAL::quote($type_codes[$i]);
			}
			$in = implode(',', $type_codes);

			// get attribute id of the price object
			$sql = 'SELECT attrid
					FROM sq_ast_attr
					WHERE type_code IN ('.$in.')
					AND name = '.MatrixDAL::quote('price_obj');

			try {
				$attrid = MatrixDAL::executeSqlAll($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to get attribute id of price object due to database error: '.$e->getMessage());
			}
			$ids = Array();
			foreach ($attrid as $row => $data) {
				$ids[] = $data['attrid'];
			}
			$in = implode(',', $ids);

			// get all assetids and their custom_vals
			$sql = 'SELECT assetid, custom_val, attrid
					FROM sq_ast_attr_val
					WHERE attrid IN ('.$in.')';

			try {
				$custom_vals = MatrixDAL::executeSqlAssoc($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to get values of existing product prices due to database error: '.$e->getMessage());
			}

			pre_echo('UPGRADING THE PRICE_OBJ ATTRIBUTE');

			// include the price class so unserialize() doesn't barf
			require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';
			// loop through all custom_vals and perform our updates
			foreach ($custom_vals as $row => $info) {
				$object = unserialize($info['custom_val']);
				if (isset($object->tax_inclusive)) {
					// if tax_inclusive was previously set, they would have wanted tax calculated
					if ($object->tax_inclusive) {
						$object->calculate_tax = TRUE;
						// add tax to the existing amount
						$object->value_has_tax = FALSE;
					} else {
						$object->calculate_tax = FALSE;
						$object->value_has_tax = FALSE;
					}
					unset($object->tax_inclusive);
				}
				$serialized = serialize($object);

				// now we can update our db with this new value
				$sql = 'UPDATE sq_ast_attr_val
						SET custom_val = '.MatrixDAL::quote($serialized).'
						WHERE attrid = '.MatrixDAL::quote($info['attrid']).'
						AND assetid = '.MatrixDAL::quote($info['assetid']);

				try {
					$result = MatrixDAL::executeSql($sql);
				} catch (Exception $e) {
					throw new Exception('Unable to update value of product due to database error: '.$e->getMessage());
				}
			}

			// lastly change the default value
			// first get the default value
			$sql = 'SELECT attrid, default_val
					FROM sq_ast_attr
					WHERE attrid IN ('.$in.')';

			try {
				$default_val = MatrixDAL::executeSqlAssoc($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to get default value of product due to database error: '.$e->getMessage());
			}

			foreach ($default_val as $row => $info) {
				// alter it
				$object = unserialize($info['default_val']);
				if (isset($object->tax_inclusive)) {
					$object->value_has_tax = $object->tax_inclusive;
					unset($object->tax_inclusive);
				}
				$serialized = serialize($object);

				$sql = 'UPDATE sq_ast_attr
						SET default_val = '.MatrixDAL::quote($serialized).'
						WHERE attrid = '.MatrixDAL::quote($info['attrid']);

				try {
					$result = MatrixDAL::executeSql($sql);
				} catch (Exception $e) {
					throw new Exception('Unable to update default value of product due to database error: '.$e->getMessage());
				}
			}

			pre_echo('UPGRADE FOR PRODUCT ASSETS COMPLETE');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>

<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_cart_edit_fns.inc,v 1.5 2005/05/25 04:33:18 lwright Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';

/**
* Ecommerce_Cart_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Listing_Engine_Edit_Fns()
	{
		$this->Listing_Engine_Edit_Fns();

	}//end Ecommerce_Cart_Edit_Fns()
	
	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object Listing_Engine		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');
		if ($asset->writeAccess('content')) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'_types', true, $display_values, null, true, true, Array('product'));
			return true;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('cms_listing_no_asset_types_chosen').'</p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
							<th><?php echo translate('inherit_question') ?></th>
						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('yes') : translate('no');
					?>
						<tr>
							<td><?php echo $type_names[$type]; ?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return false;
		}

	}//end paintAssetTypes()


}//end class

?>
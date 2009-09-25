<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api_edit_fns.inc,v 1.2 2009/09/25 00:42:49 bpearson Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Javascript API
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');

		if ($asset->writeAccess('attributes')) {

			asset_type_chooser($prefix.'_types', TRUE, array_keys($types));
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('js_api_no_asset_types_selected').'</p>';
			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('type') ?></th>
						</tr>
				<?php
				foreach ($types as $type => $value) {
					?>
						<tr>
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* When a type is removed from the listing, the type format bodycopy is
	* removed. We need to be able to have type formats for the children of all
	* our proxy asset types.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetTypes(&$asset, &$o, $prefix)
	{

		if (isset($_POST[$prefix.'_types'])) {
			$types = Array();
			foreach ($_POST[$prefix.'_types'] as $type) {
				if (!empty($type)) $types[$type] = 1;
			}
			$asset->setAttrValue('types', $types);
			return TRUE;
		}
		return FALSE;

	}//end processAssetTypes()


}//end class
?>

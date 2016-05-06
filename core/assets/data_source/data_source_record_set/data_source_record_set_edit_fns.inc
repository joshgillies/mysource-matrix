<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_source_record_set_edit_fns.inc,v 1.10 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Data_Source_Record_Set_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

        unset($this->static_screens['workflow']);
        unset($this->static_screens['metadataSchemas']);
        unset($this->static_screens['permissions']);
        unset($this->static_screens['roles']);
        unset($this->static_screens['logs']);
        unset($this->static_screens['dependants']);
        unset($this->static_screens['metadata']);
        unset($this->static_screens['settings']);
        unset($this->static_screens['tagging']);
        unset($this->static_screens['layouts']);
        unset($this->static_screens['preview']);
        unset($this->static_screens['history']);

	}//end constructor


	/**
	* Paint the interface for editing user details
	*
	* @param Data_Source_Record_Set	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintDetails(Data_Source_Record_Set $asset, Backend_Outputter $o, $prefix)
	{
		$vars = $asset->vars;

		// See if the datasource has any attributes defined as the "binary data"
		$asset_bits = explode(':', $asset->id);
		$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_bits[0]);
		$binary_attrs = $parent->getBinaryDataAttributes();
		foreach($binary_attrs as $index => $attr) {
			$binary_attrs[$index] = strtolower($attr);
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Key');?></th>
				<th><?php echo translate('Keyword');?></th>
				<th><?php echo translate('Value');?></th>
			</tr>
			<?php
			foreach ($vars as $key => $value) {
				?>
				<tr>
				<?php
				if ($key != 'shadow_asset_name') {
					?><td><?php echo $key;?></td>
					<td>
					<?php
						echo '%ds__'.$key.'%';
					?>
					</td>
					<td><?php
						// If the value is defined as "binary", do not paint it
						if (!empty($binary_attrs) && in_array($key, $binary_attrs)) {
							echo '{Binary data}';
						} else {
							echo $value;
						}
					?></td>
				<?php
				}
				?>
				</tr>
			<?php
			}
			?>
		</table>
		<?php

		return FALSE;

	}//end paintDetails()


	/**
	* Processes the interface for editing user details
	*
	* @param Data_Source_Record_Set	$asset	the asset to which we belong
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processDetails(Data_Source_Record_Set $asset, Backend_Outputter $o, $prefix)
	{
		return FALSE;

	}//end processDetails()


}//end class

?>

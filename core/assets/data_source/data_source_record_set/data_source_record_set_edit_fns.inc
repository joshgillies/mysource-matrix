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
* $Id: data_source_record_set_edit_fns.inc,v 1.3 2006/01/18 00:29:27 lwright Exp $
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
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Record_Set_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for editing user details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{
		$vars = $asset->vars;
		?>
		<b>Record Set Is: </b>
		<table>
			<?php
			foreach ($vars as $key => $value) {
				?>
				<tr>
				<?php
				if ($key != 'shadow_asset_name') {
					?><td><b><?php echo $key;?></b></td>
					<td><?php echo $value;?></td>
				<?php
				}
				?>
				</tr>
			<?php
			}
			?>
		</table>
	<?php

	}//end paintDetails()


	/**
	* Processes the interface for editing user details
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDetails(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processDetails()


}//end class

?>

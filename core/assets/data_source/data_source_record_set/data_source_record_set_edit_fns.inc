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
* $Id: data_source_record_set_edit_fns.inc,v 1.7 2007/12/02 22:37:04 lwright Exp $
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
* @version $Revision: 1.7 $
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

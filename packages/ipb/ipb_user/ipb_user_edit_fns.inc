<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_user_edit_fns.inc,v 1.5 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user_edit_fns.inc';

/**
* Ipb_User_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_User_Edit_Fns extends Backend_User_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_User_Edit_Fns()
	{
		$this->Backend_User_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint the interface for editing user details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCustomField(&$asset, &$o, $prefix)
	{
		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$custom_fields = $ipb->getCustomFields();
		$user_custom_data = $ipb->getMemberCustomFieldData($asset->id);

		if (empty($custom_fields)) {
			echo translate('no_custom_field').'<br />';
		} else {
			foreach ($custom_fields as $field) {
				$o->openField($field['pf_title']);
					if (empty($user_custom_data[0]['field_'.$field['pf_id']])) {
						echo translate('none');
					} else {
						echo $user_custom_data[0]['field_'.$field['pf_id']];
					}
				$o->closeField();
			}
		}

		return FALSE;

	}//end paintCustomField()


	/**
	* Processes the interface for editing user details
	*
	* Editing the details of an IPB user is not yet implemented
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCustomField(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processCustomField()


}//end class

?>

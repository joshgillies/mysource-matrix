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
* $Id: xml_user_bridge_edit_fns.inc,v 1.3 2007/12/21 05:03:26 gsherwood Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

/**
* xml_user_bridge_Edit_Fns
*
* Functions to handle backend interface for this asset type
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class XML_User_Bridge_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> TRUE,
													'lock_type'		=> 'attributes',
												   ),
								);

	}//end constructor


	/**
	* Paint the interface for choosing a type of user asset to use
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUserTypeChooser(&$asset, &$o, $prefix)
	{
		$user_type_codes = array_diff($GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user', TRUE), Array('root_user'));

		$user_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($user_type_codes, 'name');
		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_user_type', $user_names, FALSE, $asset->attr('user_type'));
			return TRUE;
		} else {
			echo $user_names[$asset->attr('user_type')];
			return FALSE;
		}

	}//end paintUserTypeChooser()


	/**
	* Paint the interface for choosing a type of user asset to use
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUserTypeChooser(&$asset, &$o, $prefix)
	{
		if (!empty($_POST[$prefix.'_user_type'])) {
			$user_type_codes = array_diff($GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user', TRUE), Array('root_user'));
			if (in_array($_POST[$prefix.'_user_type'], $user_type_codes)) {
				return $asset->setAttrValue('user_type', $_POST[$prefix.'_user_type']);
			}
		}
		return FALSE;

	}//end processUserTypeChooser()


	/**
	* Paint the interface for choosing a type of user asset to use
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintXMLFormat(&$asset, &$o, $prefix)
	{
		$o->openField('');
		?>
		<p>The required format for the source XML file is shown in <a href="<?php echo sq_web_path('data'); ?>/asset_types/xml_user_bridge/files/user_bridge_example.xml">this example file</a>.  The attributes of the 'user' node must include 'id' and may include any of the attributes of the asset type selected above</p>
		<?php
		$o->closeField();

	}//end paintXMLFormat()


}//end class
?>
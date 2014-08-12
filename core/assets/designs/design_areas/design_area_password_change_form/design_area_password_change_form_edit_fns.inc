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
* $Id: design_area_password_change_form_edit_fns.inc,v 1.6 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Password_Change_Form_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Password_Change_Form_Edit_Fns extends Design_Area_Edit_Fns
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
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design_Area_Password_Change_Form	$asset		the asset to which we belong
	* @param array								&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Password_Change_Form $asset, Array &$contents)
	{
		$all_contents = Array();

		// see if our page element has any sub's in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;
			if ($element['operation'] != 'password_change_section') {
				$all_contents[] = $element;
				continue;
			}

			// if there is some contents then save it to our arrays
			if (!$element['self_terminating'] && !empty($element['contents'])) {
				$all_contents = array_merge($all_contents, $element['contents']);
				if (!$asset->setAttrValue('password_change_contents', $element['contents'])) {
					return FALSE;
				}
			}

		}//end foreach

		$asset->setAttrValue('contents', $contents);

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		if (!$this->_setVariables($asset, $all_contents)) {
			return FALSE;
		}
		return $asset->saveAttributes();

	}//end _processContents()


}//end class
?>
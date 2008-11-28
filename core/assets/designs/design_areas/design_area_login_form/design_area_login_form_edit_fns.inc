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
* $Id: design_area_login_form_edit_fns.inc,v 1.18 2008/11/28 00:02:02 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Login_Form_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form_Edit_Fns extends Design_Area_Edit_Fns
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
	* @param Design_Area_Login_Form	$asset		the asset to which we belong
	* @param array					&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Login_Form $asset, Array &$contents)
	{
		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}
		$all_contents = Array();

		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;
			if ($element['operation'] != 'login_section' && $element['operation'] != 'logout_section') {
				$all_contents[] = $element;
				continue;
			}

			// get 'login' or 'logout'
			$section = substr($element['operation'], 0, -8);

			// if there is some contents then save it to our arrays
			if (!$element['self_terminating'] && !empty($element['contents'])) {
				$all_contents = array_merge($all_contents, $element['contents']);
				if (!$asset->setAttrValue($section.'_contents', $element['contents'])) {
					return FALSE;
				}
			}

		}//end foreach

		if (!$this->_setVariables($asset, $all_contents)) {
			return FALSE;
		}
		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		$asset->setAttrValue('contents', $contents);
		return $asset->saveAttributes();

	}//end _processContents()


}//end class
?>

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
* $Id: design_area_login_form_edit_fns.inc,v 1.16 2006/12/05 22:42:13 emcdonald Exp $
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
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
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
		$asset->setAttrValue('contents', $contents);
		return $asset->saveAttributes();

	}//end _processContents()


}//end class
?>
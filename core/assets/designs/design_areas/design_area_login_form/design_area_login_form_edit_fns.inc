<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: design_area_login_form_edit_fns.inc,v 1.15 2006/02/08 02:52:24 sdanis Exp $
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
* @version $Revision: 1.15 $
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
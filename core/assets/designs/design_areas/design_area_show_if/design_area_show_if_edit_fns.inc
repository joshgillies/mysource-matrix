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
* $Id: design_area_show_if_edit_fns.inc,v 1.1 2004/11/10 02:44:42 amiller Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Show_If_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Show_If_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
	{
		$then_element = false;
		$else_element = false;
		
		if (!$this->_setVariables($asset, $contents)) return false;
		
		// Check if the provided condition exists on the system
		$condition = $asset->attr('condition');
		if (!$GLOBALS['SQ_SYSTEM']->am->installed('condition_'.$condition)) {
			trigger_error('Condition '.$condition.' not found for asset "'.$asset->name.'" design area', E_USER_WARNING);
			return false;
		}
		
		// see if our asset element has any then/else elements in it
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			switch ($element['operation']) {
				case 'then' :
					if (!$then_element) {
						foreach($element['contents'] as $sub_index => $sub_element) {
							if ($sub_element['_type'] != 'TAG') continue;
							// we have sub-elements, now we just have to do something
							// with them.
						}
						
						if (!$asset->setAttrValue('then_contents', $element['contents'])) {
							return false;
						}
					} else {
						trigger_error('Cannot have more than one mysource_then section in "'.$asset->name.'" design area', E_USER_WARNING);
						return false;
					}
				
				$then_element = true;
				
				// Now we want to remove the Then section
				// array_splice($contents, $index, 1);
				break;
				
				case 'else' :
					if (!$else_element) {
						if (!$asset->setAttrValue('else_contents', $element['contents'])) {
							return false;
						}
					} else {
						trigger_error('Cannot have more than one mysource_else section in "'.$asset->name.'" design area', E_USER_WARNING);
						return false;
					}
					
					$else_element = true;
		
					// Now we want to remove the Else section
					// array_splice($contents, $index, 1);
				break;
				
			}// end switch

		}//end foreach
		
		if (!$then_element && !$else_element) {
			trigger_error('Must have at least a mysource_then or a mysource_else section in "'.$asset->name.'" design area', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = true;
		return $asset->saveAttributes();

	}//end _processContents()

}//end class

?>

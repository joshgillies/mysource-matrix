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
* $Id: design_area_login_form_edit_fns.inc,v 1.9 2004/05/07 15:48:00 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Login_Form_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Login_Form_Edit_Fns extends Design_Area_Edit_Fns
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
		if (!$this->_setVariables($asset, $contents)) return false;

		$indexes = Array('login' => -1, 'logout' => -1);
		// see if our page element has any sub's in it 
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;
			if ($element['operation'] != 'login_section' && $element['operation'] != 'logout_section') continue;

			// get 'login' or 'logout'
			$section = substr($element['operation'], 0, -8);

			$indexes[$section] = $index;

			// if there is some contents then save it to our arrays
			if (!$element['self_terminating'] && !empty($element['contents'])) {
				if (!$this->_setVariables($asset, $element['contents'])) return false;
				if (!$asset->setAttrValue($section.'_contents', $element['contents'])) return false;
			}

			// now all that is done we don't need to keep going through the array
			if ($indexes['login'] >= 0 && $indexes['logout'] >= 0) break;

		}//end foreach

		// OK, get the lowest/first index so that we can replace that one and remove the other from the contents array
		$lowest_index = count($contents);
		foreach($indexes as $i) {
			if ($i >= 0 && $lowest_index > $i) $lowest_index = $i;
		}

		foreach($indexes as $i) {
			if ($i < 0) continue;
			if ($lowest_index == $i) {
				// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
				$contents[$i] = Array(
										'_type' => 'TAG',
										'attributes'  => Array (),
										'operation' => 'sections',
										'self_terminating' => 1
									);
			// remove the other one
			} else { 
				array_splice($contents, $i, 1);
			}
		}// end foreach
	
		$asset->setAttrValue('contents', $contents);

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = true;
		return $asset->saveAttributes();

	}//end _processContents()


	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		$asset->_tmp['painting_section'] = '';
		return parent::_createDesignFileOutput($contents, $asset, $design);
		
	}//end _createDesignFileOutput()


	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'sections' :
				$login_contents  = $asset->attr('login_contents');
				$logout_contents = $asset->attr('logout_contents');
				if (empty($login_contents) && empty($logout_contents)) return true;

				echo '<', '?php 
				if (!empty($GLOBALS[\'SQ_LOGIN_FORM\'])) {
					$'.$asset->attr('id_name').'_paint_login = ($GLOBALS[\'SQ_LOGIN_FORM\'][\'paint\'] == \'login\');
				} else {
					$'.$asset->attr('id_name').'_paint_login = !$GLOBALS[\'SQ_SYSTEM\']->currentUserId();
				}
				if (', ((empty($login_contents)) ? '!' : ''), '$'.$asset->attr('id_name').'_paint_login) { 
				?', '>';

				if (!empty($login_contents)) {
					$asset->_tmp['painting_section'] = 'login';
					if (!$this->_outputContents($login_contents, $asset, $design)) return false;
					$asset->_tmp['painting_section'] = '';
				}// end if

				if (!empty($login_contents) && !empty($logout_contents)) {
					echo '<', '?php 
				} else { 
				?', '>';
				}// end if

				if (!empty($logout_contents)) {
					$asset->_tmp['painting_section'] = 'logout';
					if (!$this->_outputContents($logout_contents, $asset, $design)) return false;
					$asset->_tmp['painting_section'] = '';
				}// end if


				echo '<', '?php 
				}// end if 
				?', '>';
				return true;

			default :
				return parent::_outputElement($element, $asset, $design);
		}// end switch

	}//end _outputElement()


	/**
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see Design_Area_Edit_Fns::createParseString()
	*/
	function getSetableVars(&$asset)
	{
		$vars = parent::getSetableVars($asset);
		unset($vars['login_contents']);
		unset($vars['logout_contents']);
		return $vars;
	}// end getSetableVars()


}//end class

?>

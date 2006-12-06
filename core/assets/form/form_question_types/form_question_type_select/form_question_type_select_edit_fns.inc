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
* $Id: form_question_type_select_edit_fns.inc,v 1.3 2006/12/06 00:35:31 emcdonald Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';


/**
* Form_Question_Type_Select_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Select_Edit_Fns extends Form_Question_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Select_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();

	}//end constructor


	/**
	* paints the default answer selected by the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefault(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$options = $asset->getOptions();
		$multiple = ($asset->attr('multiple')) ? TRUE : FALSE;

		return combo_box($prefix.'_default', $options, $multiple, $asset->attr('default'));

	}//end paintDefault()


	/**
	* processes the default answer selected by the user
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_default'])) {
			$default = $_POST[$prefix.'_default'];
			if (!is_array($default)) $default = Array($default);
			if (!$asset->setAttrValue('default', $default)) {
				return FALSE;
			}
		}
		return TRUE;

	}//end processDefault()


}//end class
?>

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
* $Id: form_section_action.inc,v 1.2 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Form_Section_Action
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Action extends Asset
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
	* Paint inline interface
	*
	* @access public
	*/
	public static function paintInlineInterface(Form_Section $form_section, $settings, Backend_Outputter $o, $prefix)
	{


	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @access public
	*/
	public static function processInlineInterface(Form_Section $form_section, &$settings, Backend_Outputter $o, $prefix)
	{


	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @access public
	*/
	public static function paintSummary(Form_Section $form_section, $settings, Backend_Outputter $o, $prefix)
	{


	}//end paintSummary()


	/**
	* Check if the form section action is configured correctly
	*
	* @access public
	*/
	public static function isValid(Form_Section $form_section, $settings)
	{


	}//end isValid()

	/**
	* Execute form action
	*
	* @access public
	*/
	public static function execute(Form_Section $form_section, Form $form, $settings)
	{
	

	}//end execute()
	
	
	/**
	* If any operations needs to carried out after the last step,
	* after all the individual form sections actions are executed,
	* after the form is submitted; it will be done by this function
	*
	* Unlike form action steps i.e. execute(), this function is called 
	* only once after the form is submitted
	*
	* @access public
	*/
	public static function finalise(Form $form)
	{
	

	}//end finalise()	


}//end class

?>
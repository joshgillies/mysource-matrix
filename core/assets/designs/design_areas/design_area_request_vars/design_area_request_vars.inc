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
* $Id: design_area_request_vars.inc,v 1.3 2004/12/06 14:38:08 brobertson Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Request_Vars
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Request_Vars extends Design_Area
{

	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name
	*
	* @return mixed
	* @access public
	*/
	/*function attr($name)
	{
		if (in_array($name, Array('method','id_name'))) {
			return $this->vars[$name]['value'];
		}

		switch ($this->vars['method']['value']) {
			case 'get':
				$target_vars =& $_GET;
			break;

			case 'post':
				$target_vars =& $_POST;
			break;

			case 'request':
			default:
				$target_vars =& $_REQUEST;
			break;

		}

		return array_get_index($target_vars, $name, '');

	}*///end attr()


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		return true;
	}//end printArea()


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @access public
	*/
	function printVar($var, $args)
	{
		switch ($this->vars['method']['value']) {
			case 'get':
				$target_vars = '$_GET';
			break;

			case 'post':
				$target_vars = '$_POST';
			break;

			case 'request':
			default:
				$target_vars = '$_REQUEST';
			break;

		}

		echo '<?php echo array_get_index('.$target_vars.', \''.addslashes($var).'\', \'\') ?>';

	}//end printVar()


}//end class
?>

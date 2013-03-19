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
* $Id: design_area_request_vars.inc,v 1.11 2013/03/19 03:17:19 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Request_Vars
*
* Purpose
*	Display something from the request (GET or POST) in the design
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Request_Vars extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID of the asset to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param array	$args	array of any extra arguments that existed in the print command
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args)
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

		echo '<?php echo clean_keyword_value(array_get_index('.$target_vars.', \''.addslashes($var).'\', \'\')); ?>';

	}//end printVar()


}//end class
?>

<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_system_service.inc,v 1.1 2013/02/11 04:39:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Soap_Api_Asset_Service
*
* Purpose
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_System_Service extends Soap_Api
{


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
	    parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a function list of the asset let the children handle this
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getFunctionList()
	{
	    return Array(
				    'GetMatrixVersion'				=> '1',
			    );

	}//end getFunctionList()



	/**
	* Description: This operation returns Matrix version number
	*
	* @return void
	* @access public
	*/
	function GetMatrixVersion()
	{
			return Array (
					'MatrixVersion'	=> SQ_SYSTEM_VERSION,
				   );

	}//end GetMatrixVersion()



	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{

		$complex_types	= Array (
							'GetMatrixVersion'		=> Array (			'GetMatrixVersion'			=> Array(),

													'GetMatrixVersionResponse'		=> Array (
																					'MatrixVersion'	=> self::$string_non_optional,
																				   ),
												   ),
							
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexElements();




}//end class
?>

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
* $Id: soap_api_management.inc,v 1.6 2012/08/30 00:59:45 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Soap_Api_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class SOAP_Api_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

		$this->vars	= Array (
						'name'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The full name of the page',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'short_name'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The short name of the page',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'function_list'	=> Array (
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> '',
											'is_admin'		=> TRUE,
											'parameters'	=> Array (
																'multiple'		=> FALSE,
																'allow_empty'	=> TRUE,
															   ),
										   ),
					  );

	}//end constructor


    /**
    * Gets a list of all files that the bodycopy wants web accessable
    *
    * @return array
    * @access private
    */
    function _getFileList()
    {
        return Array(
                'web_services_caller.js',
               );

    }//end _getFileList()


}//end class
?>

<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_workflow_service_management.inc,v 1.2 2009/04/03 06:01:00 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Soap_Api_Workflow_Service_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Soap_Api_Workflow_Service_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

		$this->vars	= Array (
						'function_list'	=> Array (
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array (),
											'description'	=> '',
											'is_admin'		=> TRUE,
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
                'workflow_service_requests.js',
               );

    }//end _getFileList()


}//end class
?>

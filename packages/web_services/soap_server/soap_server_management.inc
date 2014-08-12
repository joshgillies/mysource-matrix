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
* $Id: soap_server_management.inc,v 1.4 2012/08/30 00:59:46 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Soap_Server_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Soap_Server_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);
		$this->vars['list_api']	= Array (
									'added'		=> '0.1',
									'type'		=> 'serialise',
									'is_admin'	=> FALSE,
									'default'	=> Array(),
								  );

	}//end constructor


}//end class
?>

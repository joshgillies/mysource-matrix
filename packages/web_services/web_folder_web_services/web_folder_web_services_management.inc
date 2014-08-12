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
* $Id: web_folder_web_services_management.inc,v 1.5 2012/08/30 00:59:46 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Web_Folder_Web_Services_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Web_Folder_Web_Services_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

	}//end constructor


}//end class
?>

<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: sharepoint_connector.inc,v 1.4 2012/08/30 00:59:19 ewang Exp $
*
*/





require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Sharepoint Connector
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class Sharepoint_Connector extends Asset
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
    * Returns name of the asset
    *
    * @param boolean    $short_name whether or not we are after the short name or the full name
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    function _getName($short_name=FALSE)
    {
        return $this->attr('name');

    }//end _getName()


}//end class
?>

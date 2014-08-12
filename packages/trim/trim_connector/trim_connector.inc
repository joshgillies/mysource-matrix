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
* $Id: trim_connector.inc,v 1.5 2012/08/30 00:59:28 ewang Exp $
*
*/





require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* TRIM Connector
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage trim
*/
class Trim_Connector extends Asset
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


    /**
    * Returns the proxy details, if diabled, return empty array
    *
    *
    * @return array
    * @access public
    */
    function getProxy()
    {
    	// proxy disabled
        if ($this->attr('use_proxy') == 'No') return Array();
      	$proxy = Array();
        if ($this->attr('use_proxy') == 'Yes') {
        	 //use proxy specified 
        	$proxy = Array (
        					'proxy_host' 		=> $this->attr('proxy_host'),
        					'proxy_port' 		=> (int) $this->attr('proxy_port'),
        					'proxy_login' 		=> $this->attr('proxy_login'),
        					'proxy_password' 	=> $this->attr('proxy_password'),
        			);
        }
        else {
        	//use matrix proxy
	        require_once(SQ_DATA_PATH.'/private/conf/proxy_authentication.inc');
			if (SQ_PA_ENABLED) {
				$proxy = Array (
        					'proxy_host' 		=> SQ_PA_HOSTNAME,
        					'proxy_port' 		=> (int) SQ_PA_PORT,
        					'proxy_login' 		=> SQ_PA_USERNAME,
        					'proxy_password' 	=> base64_decode(SQ_PA_PASSWORD),
        				);
			}
        }
        
        return $proxy;

    }//end getProxy()
     
}//end class
?>

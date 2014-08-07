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
* $Id: trim_connector_edit_fns.inc,v 1.5 2012/08/30 00:59:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Trim_Connector_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage trim
*/
class Trim_Connector_Edit_Fns extends Asset_Edit_Fns
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
     * Test connection
     *
     * @param object $asset
     * @param object $o
     * @param object $prefix
     *
     * @return boolean
     * @access public
     */
    function paintTestConnection(&$asset, &$o, $prefix)
    {
    	$wa = $asset->writeAccess('attributes');
    	$o->openField('Connection Status');
			require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';
			$connection	= Array (
							'wsdl'				=> $asset->attr('wsdl'),
							'authentication'	=> Array (
													'login'		=> $asset->attr('username'),
													'password'	=> $asset->attr('password'),
													'trace'		=> TRUE,
													'exception'	=> TRUE,
												   ),
						  );

			// add proxy 
			$proxy = $asset->getProxy();
			$connection['authentication'] = array_merge($connection['authentication'], $proxy);
						  
		
			$valid = FALSE;
			if (!empty($connection['wsdl'])) {
				$valid = TRIM_Common::test_trim_connection($connection);
			}//end if

			if ($valid) {
				echo '<span style="color:green;font-weight:bold">Connected</span>';
			} else {
				echo '<span style="color:red;font-weight:bold">Not Connected</span>';
			}//end if
		$o->closeField();

		return $wa;
    }//end paintTestConnection()


    /**
     * Test connection
     *
     * @param object $asset
     * @param object $o
     * @param object $prefix
     *
     * @return boolean
     * @access public
     */
    function processTestConnection(&$asset, &$o, $prefix)
    {
		return FALSE;
    }//end processTestConnection()

}//end class
?>

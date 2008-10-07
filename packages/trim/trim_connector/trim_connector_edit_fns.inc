<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: trim_connector_edit_fns.inc,v 1.2 2008/10/07 22:00:26 hnguyen Exp $
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
* @version $Revision: 1.2 $
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
												   ),
						  );

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

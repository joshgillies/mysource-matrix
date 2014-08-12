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
* $Id: sharepoint_connector_edit_fns.inc,v 1.3 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Sharepoint_Connector_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class Sharepoint_Connector_Edit_Fns extends Asset_Edit_Fns
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
			require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';
			$connection	= Array (
							'url'				=> $asset->attr('url'),
							'authentication'	=> Array (
													'login'		=> $asset->attr('username'),
													'password'	=> $asset->attr('password'),
												   ),
						  );

			$valid = FALSE;
			if (!empty($connection['url'])) {
				$valid = Sharepoint_Common::test_sharepoint_connection($connection);
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

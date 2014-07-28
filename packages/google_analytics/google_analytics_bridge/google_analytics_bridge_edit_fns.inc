<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_record_edit_fns.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token_edit_fns.inc';

/**
* Data_Record_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_Bridge_Edit_Fns extends OAuth2_Token_Edit_Fns
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
	 * Paints a list of views and a method to create a view.
	 *
	 * @param Google_Analytics_Bridge $asset  The asset being painted.
	 * @param Backend_Outputter       $o      The relevant outputter.
	 * @param string                  $prefix The prefix for form elements.
	 *
	 * @return boolean
	 */
	public function paintViewList(Google_Analytics_Bridge $asset, Backend_Outputter $o, $prefix)
	{
		$views = $asset->getViews();
		echo '<pre>';
		print_r($views);
		echo '</pre>';

		return TRUE;

	}//end paintViewList()


}//end class

?>

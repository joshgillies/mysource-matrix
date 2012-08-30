<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: db_connector_edit_fns.inc,v 1.7 2012/08/30 00:58:16 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_PACKAGES_PATH.'/data/data_source_db/data_source_db_edit_fns.inc';

/**
* Db_Connector_Edit_Fns
*
*     Edit functions for DB Connector
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Db_Connector_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens['details']['force_unlock'] = FALSE;

		unset($this->static_screens['dependants']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['tagging']);

	}//end constructor


//--        DSN        --//


	/**
	* Paints the interface for entering DB details
	*
	* @param object	&$asset	the asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintDSN(&$asset, &$o, $prefix)
	{
		$edit_fns = new Data_Source_DB_Edit_Fns();
		return $edit_fns->paintDSN($asset, $o, $prefix);

	}//end paintDSN()


	/**
	* Processes the interface for entering DSN
	*
	* @param object	&$asset	the asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDSN(&$asset, &$o, $prefix)
	{
		$edit_fns = new Data_Source_DB_Edit_Fns();
		return $edit_fns->processDSN($asset, $o, $prefix);

	}//end processDSN()


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintDSNAlone(&$asset, &$o, $prefix)
	{
		$dsn      = $asset->attr('dsn');
		$wa       = $asset->writeAccess('attributes');
		$user     = $asset->attr('user');
		$database = $asset->attr('database');
		if ($wa) {
			text_area('dsn[dsn]', empty($database) ? $dsn : '', 70, 5);
			return TRUE;
		} else {
			if (!empty($dsn)) {
				$pos = strpos($dsn, ':');
				$no_database = substr($dsn, $pos + 3, strlen($dsn) - 1);
				if (($colon_pos = strpos($no_database, ':')) !== FALSE) {
					// If the password is set, do not display it
					$at_pos = strpos($no_database, '@');
					$no_password = substr($no_database, 0, $colon_pos).substr($no_database, $at_pos, strlen($no_database) - 1);
					echo substr($dsn, 0, $pos).'://'.$no_password;
				} else {
					echo $dsn;
				}
			}
			return FALSE;
		}

	}//end paintDSNAlone()


	/**
	* Processes the interface for entering DSN
	*
	* @param object	&$asset	the asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDSNAlone(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processDSNAlone()


	/**
	* Paint Usage information for this asset
	*
	* @param object	&$asset	the asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUsageInfo(&$asset, &$o, $prefix)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'minor', 'db_connector');

		if (empty($links)) {
			echo '<span class="sq-backend-warning">'.translate('connector_not_used').'</span>';
			return TRUE;
		}

		foreach ($links as $one_link) {
			echo '<div>'.get_asset_tag_line($one_link['majorid']).'</div>';
		}

		return TRUE;


	}//end paintUsageInfo()


}//end class

?>

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
* $Id: site_prefs.inc,v 1.3 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Site_Prefs
*
* Purpose
*
*    Looks after the creation of the Site preferences
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Site_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT'	=> Array(
                                                                    'default'		=> Array(),
                                                                    'protected'		=> FALSE,
                                                                   ),
					);


	/**
	* Constructor
	*
	*/
	function __construct($pref_file='')
	{
		parent::__construct($pref_file);

		$this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['name'] = translate('Child Creation TYPE_2 Link Restriction');
		$this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['desc'] = translate('Link type options prior to asset creation will be restricted to a TYPE_2 link for any asset to be located anywhere under the specified Site Asset IDs. Link type can still be changed after creation e.g. via Linking Screen.');

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_admin  = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
	    $have_lock = $have_lock && $is_admin;

		// Paint TYPE_2 restrictions.
        $this->paintAssetMapField($o, $have_lock, $pref, 'SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT', TRUE, Array ('site' => 'I'));

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin || !$have_lock) return FALSE;

		if (is_null($pref) || $pref == 'SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT') {
			if (isset($_POST['prefs']['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT'])) {
				$site_assets = $_POST['prefs']['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT'];
				$save_assets = Array();
				foreach ($site_assets as $index => $asset_info) {
					if (!empty($asset_info['assetid'])) {
						$type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_info['assetid'], 'site');
						if (!empty($type_info)) $save_assets[] = $asset_info['assetid'];
					}//end if
				}//end foreach
				$this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['default'] = $save_assets;
			}
		}

		return TRUE;

	}//end processBackend()


}//end class

?>

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
																'name'			=> 'child_creation_type_2_restriction',
																'description'	=> 'Link type options prior to asset creation will be restricted to a TYPE 2 link for any asset to be located anywhere under the specified Site Asset IDs. Link type can still be changed after creation e.g. via Linking Screen.',
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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['name']));
			}

			if ($have_lock && $is_admin) {
				multiple_asset_finder('prefs[SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT]', $this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['default'], Array ('site' => 'I'));
			} else {
				echo '<ul>';
				foreach ($this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['default'] as $index => $assetid) {
						echo '<li>'.get_asset_tag_line($assetid).'</li>';
				}
				echo '</ul>';
			}
			echo '<div class="sq-backend-note">'.$this->pref_vars['SQ_CHILD_CREATE_LINK_TYPE_2_RESTRICT']['description'].'</div>';
			if (is_null($pref)) $o->closeField();
		}//end if

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

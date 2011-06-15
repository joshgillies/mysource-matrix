<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: file_bridge_management.inc,v 1.1.1.1.6.1 2011/06/15 05:23:02 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Bridge_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1.6.1 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class File_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'	=> Array(
							  		'added'			=> '0.1',
							  		'type'			=> 'text',
							  		'default'		=> '',
							  		'description'	=> 'The name of the bridge',
							  		'update_fn'		=> '',
							  		'is_admin'		=> '0',
							  	   ),
						'path'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The path the bridge is responsible for',
								   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			echo('UPGRADING SEARCH PAGE FROM '.$current_version."\n");
			pre_echo('Updating File Bridge...');

			// all we care is removing the XML for "webpaths" screen
			// so just do it
			$file_for_webpath = SQ_DATA_PATH.'/private/asset_types/file_bridge/localised_screens/screen_paths.en';
			if (is_file($file_for_webpath)) {
				unlink($file_for_webpath);
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Get all File Bridge assets
			$file_bridges = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('file_bridge', FALSE, TRUE);

			$count = 0;
			foreach ($file_bridges as $id => $file_bridge) {
				$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					if (!$bridge->saveWebPaths(Array(), FALSE)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

						pre_echo('UPGRADE FAILED - Could not save webpaths');
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge, TRUE);
				unset($bridge);
				$count++;

			}//end foreach bridge

			pre_echo('Updated - '.$count.' File Bridge(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


			pre_echo('FILE BRIDGES SUCCESSFULLY UPGRADED TO VERSION 0.2');
		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>

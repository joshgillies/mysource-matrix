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
* $Id: ees_login_design_management.inc,v 1.3 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* EES_Login_Design_Management
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class EES_Login_Design_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'id_name'	=> Array(
										'default'	=> 'EES Login Design',
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
		if (!parent::_upgrade($current_version)) return FALSE;
		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			// There is some css changes to default ees login design
			// if no custom changes has ever been made, we should automatically restore it to default
			pre_echo('STARTING EES LOGIN DESIGN UPGRADE - FROM VERSION 0.1');

			$new_parse_file = SQ_SYSTEM_ROOT.'/core/assets/system/ees_login_design/design_files/index.html';
			// for all possible previous versions of parse file  (different commit person, date)
			// compare md5 sum to make sure no custom changes has been made
			$source_parse_file_md5 = Array();
			$source_parse_file_md5[] = 'a0596932a27bffbd9347d3944e9711de';
			$source_parse_file_md5[] = '5ad7fd75432a2df88880779a4b2804bd';
			$source_parse_file_md5[] = 'ffdfb655681c6c05abff6163240a46af';
			$source_parse_file_md5[] = '46c990414c686b66bc8f949d124010bf';
			$source_parse_file_md5[] = 'c00cf668108207f1a41a50ab767cb42d';
			$source_parse_file_md5[] = 'c9f3ec8e7860951262e40de8e034c57c';

			$new_file = file_get_contents($new_parse_file);
			$design = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('ees_login_design');
			$design_edit_fns = $design->getEditFns();


			// Update design parse file for ees_login_design, there is some css changes
			$parse_file = $design->data_path.'/parse.txt';
			$ext_file = file_get_contents($parse_file);
			$parse_file_md5 = md5(file_get_contents($parse_file));

			$can_restore = FALSE;
			foreach ($source_parse_file_md5 as $md5key) {
			    if ($parse_file_md5 == $md5key) {
				$can_restore = TRUE;
			    }
			}
			if (!$can_restore) {
			    	echo 'Parse file of EES login design has been modified. Skipping.'."\n";
				pre_echo('EES LOGIN DESIGN UPGRADE COMPLETE - FROM VERSION 0.1');
				return TRUE;
			}
			if (!is_file($parse_file) || !is_file($new_parse_file)) {
				trigger_error ('parse file is not available');
				return FALSE;
			}
			else {

				// update the parse file
				if(!$this->_updateFile($new_parse_file, 'parse.txt', $design->data_path, $design->data_path_suffix)) {
					trigger_error('failed to update parse file '.$new_parse_file);
					return FALSE;
				}

				$design_edit_fns->parseAndProcessFile($design);
				$design->generateDesignFile();

				echo 'Parse file of EES login design is successfully updated...'."\n";
			}


			pre_echo('EES LOGIN DESIGN UPGRADE COMPLETE - FROM VERSION 0.1');

		}//end if - upgrade to version 0.2

		return TRUE;

	}//end _upgrade()


	/**
	 * _updateFile
	 * Update the target file by removing it first and then replace with a new file, update file versioning of it
	 *
	 * @return Boolean
	 */
	function _updateFile ($new_file, $file_name, $data_path, $data_path_suffix) {
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		$file_path = $data_path.'/'.$file_name;

		if (!unlink($file_path)) {
			trigger_error('failed to remove old file '.$file_name);
			return FALSE;
		}

		if (string_to_file(file_get_contents($new_file), $file_path)) {
			// add a new version to the repository
			$file_status = $fv->upToDate($file_path);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($file_path, '')) {
					trigger_localised_error('CORE0160', translate('Failed committing file version'), E_USER_WARNING);
				}
			}
		} else {
			trigger_error('Can not overwrite old file '.$file_name);
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($data_path_suffix.'/'.$file_name, $data_path)) {
			trigger_localised_error('CORE0032', translate('Failed checking out latest version of file'), E_USER_WARNING);
			return FALSE;
		}//end if


		return TRUE;
	}//end _updateFile


}//end class
?>

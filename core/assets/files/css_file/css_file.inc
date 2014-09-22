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
* $Id: css_file.inc,v 1.6 2012/08/30 01:09:06 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* CSS_File
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class CSS_File extends File
{


	/**
	* All allowed extensions for a css
	*
	* @var Array(string)
	*/
	public $allowed_extensions = Array('css');


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
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	* @param array	$info	info about the uploaded file
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link, $info=Array())
	{
		$this->_tmp['file_create_data'] =& $info;

		if (empty($info)) {
			// Try getting the uploaded file details
			$info = get_file_upload_info($this->getPrefix());
		}

		if ($info === FALSE || empty($info)) {
			// See if they've chosen a pre-uploaded file
			$edit = $this->getEditFns();
			$info = $edit->getChosenFileInfo($this->getPrefix());
		}

		$file_name = '';
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {

			// When no file is supplied via uploaded, create a new file
			if ($info === FALSE || empty($info)) {

				$file_title = trim($this->attr('title'));
				$file_title = preg_replace('/\s/', '_', $file_title);
				$file_title = preg_replace('/[^A-Za-z0-9_\-]/', '', $file_title);

				// Take first name from the allowed extension names array for this file
				$extension = $this->allowed_extensions[0];

				$file_name = $file_title . '.' . $extension;

				$create_location = SQ_SYSTEM_ROOT.'/'.'data/temp';
				// Create an temporary empty file
				if (!fopen($create_location.'/'.$file_name, 'w+')) {
					trigger_localised_error('SYS0025', sprintf(translate('Unable to create file: %s'), $file_name. " from given title ".$file_title), E_USER_WARNING)
					return FALSE;
				}

				// Create info array for the file just created

				$info = Array();

				$info['name'] = $file_name;
				$info['tmp_name'] = $create_location;
				$info['created_file'] = TRUE;
			}
		}

		// Make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			if (!$this->validFile($info)) return FALSE;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			// Check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($info['name']));
			$name = array_shift($valid_names);

			// Make grep sure the new web path is not already is use
			$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
			if (!empty($bad_paths)) {
				trigger_localised_error('CORE0086', sprintf(translate('Unable to create file with name "%s", web path already exists?'), $name), E_USER_WARNING)
				return FALSE;
			}

			$this->setAttrValue('name', $name);
		}//end if

		return parent::create($link, $info);

	}//end create()

}//end class
?>

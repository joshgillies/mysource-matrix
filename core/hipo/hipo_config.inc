<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: hipo_config.inc,v 1.6 2003/11/17 00:57:05 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* HIPO_Config
*
* Purpose
*
*    Looks after the creation and editing of the hipo config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class HIPO_Config extends Config
{
	var $config_vars = Array(
		'SQ_HIPO_USE_SERVER'		=> Array('editable' => 0, 'default' => false),
		'SQ_HIPO_SERVER_DOMAIN'		=> Array('editable' => 0, 'default' => ''),
		'SQ_HIPO_SERVER_PORT'		=> Array('editable' => 0, 'default' => 0),
		'SQ_HIPO_SERVER_SYSTEMID'	=> Array('editable' => 0, 'default' => ''),
		'SQ_HIPO_TOTAL_THRESHOLD'	=> Array('editable' => 1, 'default' => 80),
	);

	/**
	* Constructor
	*
	*/
	function HIPO_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/hipo.inc';
		$hipo_job_path = SQ_SYSTEM_ROOT.'/core/hipo/jobs';

		// ask each of the jobs for their config vars
		if (is_dir($hipo_job_path)) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$d = dir($hipo_job_path);
			while (false !== ($file = $d->read())) {
				if ($file == '.' || $file == '..') continue;
				// skip hidden files
				if (strpos($file, '.') == 0) continue;

				if (!is_dir($hipo_job_path.'/'.$file)) {
					require_once $hipo_job_path.'/'.$file;
					if (get_file_type($file) != 'inc') continue;
					$type_code = substr($file, 0, strpos($file, '.'));

					if ($type_code) {
						eval('$vars = '.$type_code.'::getConfigVars();');
						$this->config_vars = array_merge($this->config_vars, $vars);
					}
				}
			}
			
			$d->close();
		}
	}

}//end class

?>

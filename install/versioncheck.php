<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
* 
* $Id: versioncheck.php,v 1.1 2005/03/16 04:24:46 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/

$step1 = &new PHPCheckAction();
$step2 = &new PEARCheckAction();

if (!$step1->processAction()) {
	foreach ($step1->required_settings as $app => $setting) {
		if (!$setting['available']) {
			echo str_pad("\t".$app, 40);
			echo "[ FAILED ]\n";
		}
	}
	exit(1);
}

foreach ($step1->required_settings as $app => $setting) {
	echo str_pad("\t".$app, 40);
	echo "[ OK ]\n";
}

// make a guess and prompt user for PEAR path ?
if (!$step2->processAction()) {
	foreach ($step2->required_packages as $app => $setting) {
		if (!$setting['available']) {
			echo str_pad("\t".$app, 40);
			echo "[ FAILED ]\n";
		}
	}
	exit(1);
}

foreach ($step2->required_packages as $app => $setting) {
	echo str_pad("\t".$app, 40);
	echo "[ OK ]\n";
}

/**
* PHPCheckAction
* 
* Perform checks for the PHP version
* 
*
* @author mnyeholt
* @version $version$ - 1.0
* @package MySource_Matrix
* @subpackage installer
*/
class PHPCheckAction 
{
	/**
	* The minumum version that Matrix will install on.
	* @var type
	*/
	var $min_version = '4.3.0';
	
	/**
	* Holds an array about what requirements are needed.
	* array( setting => Array('version'=>vernum, 'available'=>boolean, 'message' => string) )
	* @var array
	*/
	var $required_settings = Array(	'PHP' => Array('version'=>'4.3.2', 'available'=>true, 'message'=> ''),
										'GD'=>Array('version'=>'', 'available' => true, 'message'=> ''),
										'Zlib'=>Array('version'=>'', 'available' => true, 'message'=> ''),
										'PSpell'=>Array('version'=>'', 'available' => true, 'message'=> '')
									);
	
	
	/**
	* Figure out whether the current PHP version is sufficient
	* 
	* @return boolean Whether this check succeeded or not.
	* @access public
	*/
	function processAction()
	{
		
		$this->success = true;
		if(version_compare(phpversion(), $this->required_settings['PHP']['version']) < 0) {
			$this->success = false;
			$this->required_settings['PHP']['available'] = false;
			$this->required_settings['PHP']['message'] =  '<img src="./extra/failed.png" border="0"  alt="Failed" title="Failed" />';
		} else {
			$this->required_settings['PHP']['message'] =  '<img src="./extra/ok.png" border="0" alt="OK" title="OK"  />';
		}
		
		// check if gd available
		if (!function_exists('gd_info')) {
			$this->required_settings['GD']['available'] = false;
			$this->required_settings['GD']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			$this->errors[] = 'You must have GD installed to continue.';
			$this->success = false;
		} else {
			$this->required_settings['GD']['message'] =  '<img src="./extra/ok.png" border="0" alt="OK" title="OK"  />';
		}
		
		// check zlib
		if (!function_exists('gzopen')) {
			$this->required_settings['Zlib']['available'] = false;
			$this->required_settings['Zlib']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			$this->success = false;
		} else {
			$this->required_settings['Zlib']['message'] =  '<img src="./extra/ok.png" border="0" alt="OK" title="OK"  />';
		}
		
		// check pspell
		if (!function_exists('pspell_check')) {
			$this->required_settings['PSpell']['available'] = false;
			$this->required_settings['PSpell']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			$this->errors[] = 'PSpell not found. You can proceed with the install, however some functionality will not be available to you';
		} else {
			$this->required_settings['PSpell']['message'] =  '<img src="./extra/ok.png" border="0" alt="OK" title="OK"  />';
		}
		
		return $this->success;
		
	}//end processAction()
	
	
	/**
	* Display the result of this check.
	* 
	* @param
	* @return void
	* @access public
	*/
	function paintAction()
	{
		
		
	}//end paintAction()
	
	
	/**
	* Show the confirmation of what passed and what failed.
	* 
	* @return void
	* @access public
	*/
	function confirmAction()
	{
		$tpl = new Template(INSTALL_DIR.'/templates/versioncheck.tpl');
		$tpl->set('step_name', $this->parent_step);
		$tpl->set('checking', 'PHP Modules');
		$tpl->set('required', $this->required_settings);
		
		echo $tpl->fetch();
		
	}//end confirmAction()
	
	
}//end class


/**
* PEARCheckAction
* 
* Check whether all the PEAR versions are up to date and available.
*
* @author gsherwood
* @version $version$ - 1.0
* @package MySource_Matrix
* @subpackage installer
*/
class PEARCheckAction 
{
	
	/**
	* The required packages for PEAR
	* @var array
	*/
	var $required_packages = Array(
									'DB'          	=> Array('version'=>'1.6.2','available'=>false,'message' => ''),
									'Archive_Tar' 	=> Array('version'=>'1.1','available'=>false,'message' => ''),
									'Mail'        	=> Array('version'=>'1.1.3','available'=>false,'message' => ''),
									'Mail_Mime'   	=> Array('version'=>'1.2.1','available'=>false,'message' => ''),
									'XML_HTMLSax' 	=> Array('version'=>'2.1.2','available'=>false,'message' => ''),
									'XML_Parser'  	=> Array('version'=>'1.0.1','available'=>false,'message' => ''),
									'Cache'  		=> Array('version'=>'1.5.3','available'=>false,'message' => ''),
									'HTTP_Client'  	=> Array('version'=>'1.0.0','available'=>false,'message' => ''),
								  );
	
	
	/**
	* Check the pear versions
	* 
	* @return boolean Whether this check succeeded or not.
	* @access public
	*/
	function processAction()
	{
		$this->success = true;
		$pear_path = '';
		// check if the pear classes exist
		if ($pear_path == '') {
			$this->errors[] = 'Could not find the PEAR registry. You can still proceed with install, '.
								'but you may encounter some errors.';
			
			// go through and manually try and instantiate PEAR classes.
			@include_once 'DB.php';
			if (!class_exists('DB')) {
				$this->errors[] = 'Could not find PEAR::DB';
				$this->required_packages['DB']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['DB']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'XML/XML_HTMLSax.php';
			if (!class_exists('XML_HTMLSax')) {
				$this->errors[] = 'Could not find PEAR::XML_HTMLSax';
				$this->required_packages['XML_HTMLSax']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Help"  />';
			} else {
				$this->required_packages['XML_HTMLSax']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'Archive/Tar.php';
			if (!class_exists('Archive_Tar')) {
				$this->errors[] = 'Could not find PEAR::Archive_Tar';
				$this->required_packages['Archive_Tar']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Help"  />';
			} else {
				$this->required_packages['Archive_Tar']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'Mail.php';
			if (!class_exists('Mail')) {
				$this->errors[] = 'Could not find PEAR::Mail';
				$this->required_packages['Mail']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['Mail']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'Mail/mime.php';
			if (!class_exists('Mail_mime')) {
				$this->errors[] = 'Could not find PEAR::Mail_Mime';
				$this->required_packages['Mail_Mime']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['Mail_Mime']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'XML/Parser.php';
			if (!class_exists('XML_Parser')) {
				$this->errors[] = 'Could not find PEAR::XML_Parser';
				$this->required_packages['XML_Parser']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['XML_Parser']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'Cache.php';
			if (!class_exists('Cache')) {
				$this->errors[] = 'Could not find PEAR::Cache';
				$this->required_packages['Cache']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['Cache']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
			
			@include_once 'HTTP/Client.php';
			if (!class_exists('HTTP_Client')) {
				$this->errors[] = 'Could not find PEAR::HTTP_Client';
				$this->required_packages['HTTP_Client']['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
			} else {
				$this->required_packages['HTTP_Client']['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
			}
						
		} else { //end if registry not found
			include_once 'PEAR/Registry.php';
			$pear_registry = new PEAR_Registry($_SESSION['pear_path']); 
			
			if ($pear_registry == null) {
				$this->errors[] = 'PEAR Registry could not be found.';
				$this->success = false;
				return false;
			}
			foreach ($this->required_packages as $package_name => $required_version) {
				$package_info = $pear_registry->packageInfo($package_name);
				if (empty($package_info)) {
					$this->errors[] = 'Required PEAR module "'.$package_name.'" is not installed. Please run ';
					$this->errors[] = '<pre>pear install '.$package_name;
					$this->required_packages[$package_name]['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
					$this->success = false;
				} else {
					if (version_compare($required_version['version'], $package_info['version']) < 0) {
						$this->required_packages[$package_name]['available'] = true;
						$this->required_packages[$package_name]['message'] =  '<img src="./extra/yellow_help_on.png" border="0" alt="Help" title="Help"  />';
						$this->errors[] = 'Your version of '.$package_name.' meets the requirements, but has '.
						'not been fully tested with Matrix and may cause some undesirable side-effects.';
						
					} elseif (version_compare($required_version['version'], $package_info['version']) == 0) {
						$this->required_packages[$package_name]['available'] = true;
						$this->required_packages[$package_name]['message'] =  '<img src="./extra/ok.png" border="0" alt="OK" title="OK"  />';
					} else {
						$this->required_packages[$package_name]['message'] =  '<img src="./extra/failed.png" border="0" alt="Failed" title="Failed"  />';
						$this->errors[] = 'Please run <pre>pear upgrade '.$package_name.'</pre>';
						$this->success = false;
					}
				}
			}
		}// end if registry was found
		

		return $this->success;
		
	}//end processAction()
	
	
	/**
	* Display whether the PEAR versions where up to date or not.
	* 
	* @return void
	* @access public
	*/
	function paintAction()
	{
		$tpl = new Template(INSTALL_DIR.'/templates/versioncheck.tpl');
		
		$tpl->set('step_name', $this->parent_step);
		$tpl->set('required', $this->required_packages);
		$tpl->set('checking', 'PEAR Packages');
		
		echo $tpl->fetch();
									
		$pear_dir  = isset($_SESSION['pear_path']) ? $_SESSION['pear_path'] : $this->_findPEARPath();
		
		$pear_dir = 
		
		$textbox = '<p>Please enter your PEAR path. The installer has guessed that the directory below is '.
					'where PEAR is installed, but it may be in a different directory. '.
					'PEAR by default is installed underneath your PHP directory in /path/to/php/PEAR.</p>'.
					'<p>If the installer cannot find the PEAR registry, set this value blank before clicking '.
					'next, and the installer will try to detect the classes manually. You will then be able '.
					'to continue installing regardless of whether the PEAR classes were found.</p>'.
					'<input type="text" name="pear_path" size="40" value="'.$pear_dir.'" />';
		echo $textbox;
		
	}//end paintAction()
	
	
	/**
	* Show the confirmation of what passed and what failed.
	* 
	* @return void
	* @access public
	*/
	function confirmAction()
	{
		$tpl = new Template(INSTALL_DIR.'/templates/versioncheck.tpl');
		
		$tpl->set('checking', 'PEAR Packages');
		$tpl->set('step_name', $this->parent_step);
		$tpl->set('required', $this->required_packages);
		
		echo $tpl->fetch();
		
	}//end confirmAction()
	
	
	/**
	* Try and find the PEAR path.
	* 
	* @return string the likely PEAR path.
	* @access private
	*/
	function _findPEARPath()
	{
		$locations = Array('/usr/lib/php/PEAR', 
									'/usr/lib/php', 
									'/usr/local/lib/php',
									'/usr/local/lib/php/PEAR',
									'c:/php/pear',
									'c:/php4/pear'
									);
									
		foreach ($locations as $location) {
			// if the location is a directory, see if there's a PEAR.php file there.
			if(is_dir($location)) {
				$pear_file = $location.'/PEAR.php';
				if(file_exists($pear_file)) {
					return $location;
				}
			}
			
		}
		
		$pear_dir = '';
		// if none of them were found, pass back a guess.
		/*if ((strtolower(PHP_OS) == 'winnt') || (strtolower(PHP_OS) == 'win32')) {
			$pear_dir = 'c:/php/pear';
		} else {
			$pear_dir = '/usr/local/lib/php';
		}*/
		
		return $pear_dir;
		
	}//end _findPEARPath()
	
	
}//end class
?>

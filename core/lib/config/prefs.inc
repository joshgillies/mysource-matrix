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
* $Id: prefs.inc,v 1.2 2004/06/18 13:17:55 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Prefs
*
* Purpose
*
*    Looks after the creation and editing of the user preference files
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Prefs extends MySource_Object
{

	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $pref_vars = Array();

	/**
	* The file to save the config file to
	* @var string
	*/
	var $pref_file = '';


	/**
	* Constructor
	*
	*/
	function Prefs($pref_file='')
	{
		$this->MySource_Object();

		// include the correct preference file
		if (empty($pref_file)) $this->pref_file = SQ_DATA_PATH.'/private/conf/preferences.inc';
		else $this->pref_file = $pref_file;

		if (is_file($this->pref_file)) {
			include $this->pref_file;

			$pref_type = substr(get_class($this), 0, -6);
			if (isset($preferences[$pref_type])) {
				foreach ($preferences[$pref_type] as $var_name => $var_data) {
					if (!isset($this->pref_vars[$var_name])) continue;
					$this->pref_vars[$var_name]['default']   = $var_data['default'];
					$this->pref_vars[$var_name]['protected'] = $var_data['protected'];
				}
			}
		}

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=null)
	{
		return false;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=null)
	{
		return false;

	}//end processBackend()


	/**
	* Merges two preference arrays for a content type
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		// by default, just return the first preferences that have been set
		// because we have no idea how to merge the two
		return $prefs1;

	}//end mergePrefs()


}//end class

?>
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
* $Id: prefs.inc,v 1.8 2012/08/30 01:09:21 ewang Exp $
*
*/


/**
* Prefs
*
* Purpose
*
*    Looks after the creation and editing of the user preference files
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
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

			$pref_type = substr((get_class_lower($this)), 0, -6);
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
	* Merges two group-defined preference arrays for a content type
	*
	* Note that this function is only called if there is a conflict between two groups' preferences.
	* If there is a conflict between a group's prefs and the global prefs, the group automatically wins and
	* this function is not called.
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array	Prefs for this asset type, prefs_var => prefs_details
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

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
* $Id: bodycopy_container_prefs.inc,v 1.2.2.1 2006/03/16 23:17:03 rong Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Bodycopy_Container_Prefs
*
* Purpose
*
*    Looks after the creation the WYSIWYG preferences
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2.2.1 $
* @package MySource_Matrix
*/
class Bodycopy_Container_Prefs extends Prefs
{
	var $pref_vars = Array(
						'SQ_DEFAULT_CONTENT_TYPE'	=> Array(
														'name'			=> 'Default Content Type',
														'description'	=> 'When bodycopy containers are created they will have this content type by default',
														'default'		=> 'content_type_wysiwyg',
														'protected'		=> FALSE,
													   ),
					 );


	/**
	* Constructor
	*
	*/
	function Bodycopy_Container_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter instance
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

		if (is_null($pref) || $pref == 'SQ_DEFAULT_CONTENT_TYPE') {
			if (is_null($pref)) {
				$o->openField(translate('default_content_type'));
			}

				$default_content_type = $this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['default'];
				if ($have_lock) {
					$all_content_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('content_type');
					$content_type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($all_content_types, 'name');
					combo_box('prefs[SQ_DEFAULT_CONTENT_TYPE]', $content_type_names, FALSE, $default_content_type);
					echo '<div class="sq-backend-note">'.translate('default_content_type_note').'</div>';
				} else {
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($default_content_type, 'name');
				}

			if (is_null($pref)) $o->closeField();
		}

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter instance
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_DEFAULT_CONTENT_TYPE') {
			if (isset($_POST['prefs']['SQ_DEFAULT_CONTENT_TYPE'])) {
				$this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['default'] = $_POST['prefs']['SQ_DEFAULT_CONTENT_TYPE'];
			}
		}

		return TRUE;

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
		$merged_prefs = Array();
		$pref_vars = Array('SQ_DEFAULT_CONTENT_TYPE');

		foreach ($pref_vars as $var_name) {
			if (isset($prefs1[$var_name]) && !isset($prefs2[$var_name])) {
				// this is only set in prefs1
				$merged_prefs[$var_name] = $prefs1[$var_name];
			} else if (isset($prefs2[$var_name]) && !isset($prefs1[$var_name])) {
				// this is only set in prefs2
				$merged_prefs[$var_name] = $prefs2[$var_name];
			} else if (isset($prefs1[$var_name]) && isset($prefs2[$var_name])) {
				// this is set in both prefs
				switch ($var_name) {
					case 'SQ_DEFAULT_CONTENT_TYPE' :
						// prefer non-wysiwyg
						if ($prefs1[$var_name] == 'content_type_wysiwyg') {
							$merged_prefs[$var_name] = $prefs2[$var_name];
						} else {
							$merged_prefs[$var_name] = $prefs1[$var_name];
						}
					break;
				}

				$merged_prefs[$var_name]['name']		= $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description']	= $prefs1[$var_name]['description'];
				if (isset($prefs2[$var_name]['protected'])) {
					$merged_prefs[$var_name]['protected'] = ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
				} else {
					$merged_prefs[$var_name]['protected'] = $prefs1[$var_name]['protected'];
				}
			}
		}

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>
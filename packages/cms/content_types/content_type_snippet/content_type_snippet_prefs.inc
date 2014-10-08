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
* $Id: content_type_snippet_prefs.inc,v 1.5 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Content_Type_Snippet_Prefs
*
* Purpose
*
*    Snippet root node preference
*
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Content_Type_Snippet_Prefs extends Prefs
{
	var $pref_vars = Array(
						'SQ_SNIPPET_ROOT'	=> Array(
												'default'		=> '',
												'protected'		=> FALSE,
											   ),
						'SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK'	=> Array(
												'default'		=> '1',
												'protected'		=> FALSE,
											   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_Snippet_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

		$this->pref_vars['SQ_SNIPPET_ROOT']['name'] = translate('Snippet Root Node');
		$this->pref_vars['SQ_SNIPPET_ROOT']['desc'] = translate('All content types found under this asset will be available in the Snippet Content Type dropdown.');

		$this->pref_vars['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['name'] = translate('Snippet Keywords Permission Check');
		$this->pref_vars['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['desc'] = translate('Turn off WYSIWYG snippet keywords permission check to improve performance.');

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter object
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

		$this->paintAssetMapField($o, $have_lock, $pref, 'SQ_SNIPPET_ROOT');

        $options = Array(
                    1 => translate('Yes'),
                    0 => translate('No'),
                   );
		$this->paintComboField($o, $have_lock, $pref, 'SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK', $options);

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter object
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
		if (is_null($pref) || $pref == 'SQ_SNIPPET_ROOT') {
			// update snippet root node pref
			if (isset($_POST['prefs']['SQ_SNIPPET_ROOT'])) {
			    $this->pref_vars['SQ_SNIPPET_ROOT']['default'] = $_POST['prefs']['SQ_SNIPPET_ROOT']['assetid'];
			}
		}

		// Allows Turn Off Keywords Permission Check
		if (is_null($pref) || $pref == 'SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK') {
			if (isset($_POST['prefs']['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK'])) {
				$allow = strtoupper(trim($_POST['prefs']['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']));
				$this->pref_vars['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default'] = $allow;
			}
		}

		return TRUE;

	}//end processBackend()


	/**
	* Merges two group-defined preference arrays for content type snippet
	*
	* Note that this function is only called if there is a conflict between two groups' preferences.
	* If there is a conflict between a group's prefs and the global prefs, the group automatically wins and
	* this function is not called.
	* Array returned is of the format (prefs_var => prefs_details)
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		// Combine the root nodes in the preferences
		if (isset($prefs1['SQ_SNIPPET_ROOT']['default']) && isset($prefs2['SQ_SNIPPET_ROOT'])) {
			if (is_array($prefs2['SQ_SNIPPET_ROOT']) && isset($prefs2['SQ_SNIPPET_ROOT']['default'])) {
				$prefs1['SQ_SNIPPET_ROOT']['default'] .= ','.$prefs2['SQ_SNIPPET_ROOT']['default'];
			} else {
				$prefs1['SQ_SNIPPET_ROOT']['default'] .= ','.$prefs2['SQ_SNIPPET_ROOT'];
			}
		}

		// If permission check is enabled in any prefernce, permission check will be enabled for all
		if (isset($prefs1['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default']) && isset($prefs2['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK'])) {
			if (is_array($prefs2['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']) && isset($prefs2['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default'])) {
				if ($prefs2['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default']) {
					$prefs1['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default'] = 1;
				}
			} else {
				if ($prefs2['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']) {
					$prefs1['SQ_SNIPPET_KEYWORDS_PERMISSION_CHECK']['default'] = 1;
				}
			}
		}

		return parent::mergePrefs($prefs1, $prefs2);

	}//end mergePrefs()

}//end class
?>

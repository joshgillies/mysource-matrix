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
* $Id: bodycopy_container_prefs.inc,v 1.10 2012/08/30 01:09:05 ewang Exp $
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
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/
class Bodycopy_Container_Prefs extends Prefs
{
	public $pref_vars = Array(
							'SQ_DEFAULT_CONTENT_TYPE'		=> Array(
																'default'		=> 'content_type_wysiwyg',
																'protected'		=> FALSE,
															   ),
							'SQ_DEFAULT_PRESENTATION_TYPE'	=> Array(
																'default'		=> 'div',
																'protected'		=> FALSE,
															),
							'SQ_DIV_DISABLE_CREATE'	=> Array(
																'default'		=> 'no',
																'protected'		=> FALSE,
															),
							'SQ_DIV_DISABLE_DELETE'	=> Array(
																'default'		=> 'no',
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

		$this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['name'] = translate('Default Content Type');
        $this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['desc'] = translate('When new Content Containers are created they will use this content type.');

		$this->pref_vars['SQ_DEFAULT_PRESENTATION_TYPE']['name'] = translate('Default Presentation Type');
        $this->pref_vars['SQ_DEFAULT_PRESENTATION_TYPE']['desc'] = translate('When new Content Containers are created they will use this presentation type. by default. This can be changed for a particular Content Container at any time.');

        $this->pref_vars['SQ_DIV_DISABLE_CREATE']['name'] = translate('Creating Containers on Contents Screen');
        $this->pref_vars['SQ_DIV_DISABLE_CREATE']['desc'] = translate('Allows you to stop users from being able to create Content Containers on the Edit Contents screen. The users, however, will still be able to move, link, clone and delete them in the Asset Map.');

        $this->pref_vars['SQ_DIV_DISABLE_DELETE']['name'] = translate('Deleting Containers on Contents Screen');
        $this->pref_vars['SQ_DIV_DISABLE_DELETE']['desc'] = translate('Allows you to stop users from being able to delete Content Containers on the Edit Contents screen. The users, however, will still be able to move, link, clone and delete them in the Asset Map.');

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param Backend_Outputter	$o			reference to the backend outputter instance
	* @param boolean			$have_lock	do we have the lock to edit these preferences or not
	* @param string				$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o, $have_lock, $pref=NULL)
	{
	    $is_admin  = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
	    $have_lock = $have_lock && $is_admin;

	    // Default content type.
		$all_content_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('content_type');

        // If markdown is not enabled, remove it from the list
        $markdown_key = array_search('content_type_markdown', $all_content_types);
        if ($markdown_key !== FALSE) {
            $disable_markdown = TRUE;
            require_once SQ_DATA_PATH.'/private/conf/tools.inc';
            if (SQ_TOOL_MARKDOWN_ENABLED && is_file(SQ_TOOL_MARKDOWN_PATH)) {
                include_once SQ_TOOL_MARKDOWN_PATH;
                if (function_exists('Markdown')) {
                    $disable_markdown = FALSE;
                }
            }
            if ($disable_markdown) {
                unset($all_content_types[$markdown_key]);
            }
        }

        $content_type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($all_content_types, 'name');
		$this->paintComboField($o, $have_lock, $pref, 'SQ_DEFAULT_CONTENT_TYPE', $content_type_names);

	    // Default presentation type.
        $possible_types = Array(
                            'div'	=> translate('Block (div)'),
                            'span'	=> translate('Inline (span)'),
                            'none'	=> translate('Raw (no formatting)'),
                          );
		$this->paintComboField($o, $have_lock, $pref, 'SQ_DEFAULT_PRESENTATION_TYPE', $possible_types);

		// Disable create and delete.
        $options = Array(
                    'yes' => translate('Disable creation'),
                    'no'  => translate('Allow creation'),
                   );
		$this->paintComboField($o, $have_lock, $pref, 'SQ_DIV_DISABLE_CREATE', $options);

        $options = Array(
                    'yes' => translate('Disable deletion'),
                    'no'  => translate('Allow deletion'),
                   );
		$this->paintComboField($o, $have_lock, $pref, 'SQ_DIV_DISABLE_DELETE', $options);

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param Backend_Outputter	$o			reference to the backend outputter instance
	* @param boolean			$have_lock	do we have the lock to edit these preferences or not
	* @param string				$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Backend_Outputter $o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_DEFAULT_CONTENT_TYPE') {
			if (isset($_POST['prefs']['SQ_DEFAULT_CONTENT_TYPE'])) {
				$this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['default'] = $_POST['prefs']['SQ_DEFAULT_CONTENT_TYPE'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_DEFAULT_PRESENTATION_TYPE') {
			if (isset($_POST['prefs']['SQ_DEFAULT_PRESENTATION_TYPE'])) {
				$this->pref_vars['SQ_DEFAULT_PRESENTATION_TYPE']['default'] = $_POST['prefs']['SQ_DEFAULT_PRESENTATION_TYPE'];
			}
		}

		// Div DISABLE CREATE
		if (is_null($pref) || $pref == 'SQ_DIV_DISABLE_CREATE') {
			if (isset($_POST['prefs']['SQ_DIV_DISABLE_CREATE'])) {
				$this->pref_vars['SQ_DIV_DISABLE_CREATE']['default'] = $_POST['prefs']['SQ_DIV_DISABLE_CREATE'];
			}
		}

		// Div DISABLE DELETE
		if (is_null($pref) || $pref == 'SQ_DIV_DISABLE_DELETE') {
			if (isset($_POST['prefs']['SQ_DIV_DISABLE_DELETE'])) {
				$this->pref_vars['SQ_DIV_DISABLE_DELETE']['default'] = $_POST['prefs']['SQ_DIV_DISABLE_DELETE'];
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
	public function mergePrefs(Array $prefs1, Array $prefs2)
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

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
																'name'			=> 'default_content_type',
																'description'	=> 'When bodycopy containers are created they will have this content type by default',
																'default'		=> 'content_type_wysiwyg',
																'protected'		=> FALSE,
															   ),
							'SQ_DEFAULT_PRESENTATION_TYPE'	=> Array(
																'name'			=> 'default_presentation_type',
																'description'	=> 'When bodycopy containers are created they will have this presentation type by default',
																'default'		=> 'div',
																'protected'		=> FALSE,
															),
							'SQ_DIV_DISABLE_CREATE'	=> Array(
																'name'			=> 'div_disable_create',
																'description'	=> 'If bodycopy containers are disabled for creating, they can not be created in Edit Content Screen',
																'default'		=> 'no',
																'protected'		=> FALSE,
															),
							'SQ_DIV_DISABLE_DELETE'	=> Array(
																'name'			=> 'div_disable_delete',
																'description'	=> 'If bodycopy containers are disabled for deleting, they can not be deleted in Edit Content Screen',
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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_DEFAULT_CONTENT_TYPE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['name']));
			}

				$default_content_type = $this->pref_vars['SQ_DEFAULT_CONTENT_TYPE']['default'];
				if ($have_lock) {
					$all_content_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('content_type');

					//if markdown is not enabled, remove it from the content type
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
					combo_box('prefs[SQ_DEFAULT_CONTENT_TYPE]', $content_type_names, FALSE, $default_content_type);
					echo '<div class="sq-backend-note">'.translate('default_content_type_note').'</div>';
				} else {
					echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($default_content_type, 'name');
				}

			if (is_null($pref)) $o->closeField();
		}

		if (is_null($pref) || $pref == 'SQ_DEFAULT_PRESENTATION_TYPE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_DEFAULT_PRESENTATION_TYPE']['name']));
			}

				$default_pres_type = $this->pref_vars['SQ_DEFAULT_PRESENTATION_TYPE']['default'];
				$possible_types = Array(
									'div'	=> translate('block-level'),
									'span'	=> translate('inline'),
									'none'	=> translate('raw_html'),
								  );

				if ($have_lock) {
					combo_box('prefs[SQ_DEFAULT_PRESENTATION_TYPE]', $possible_types, FALSE, $default_pres_type);
					echo '<div class="sq-backend-note">'.translate('default_presentation_type_note').'</div>';
				} else {
					echo $possible_types[$default_pres_type];
				}

			if (is_null($pref)) $o->closeField();
		}


		if (is_null($pref) || $pref == 'SQ_DIV_DISABLE_CREATE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_DIV_DISABLE_CREATE']['name']));
			}
				$div_disable_create = $this->pref_vars['SQ_DIV_DISABLE_CREATE']['default'];
				?>
				<table class="sq-backend-table" style="width: 100px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap">
						<?php
							echo translate('disable');
						?>
						</td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							combo_box('prefs[SQ_DIV_DISABLE_CREATE][default]', Array('yes' => 'Yes', 'no' => 'No'), FALSE, $div_disable_create);
						} else {
							echo ucfirst($div_disable_create);
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			$o->note(translate('div_disable_create_note'));

			if (is_null($pref)) $o->closeField();

		}//end if show SQ_DIV_DISABLE_CREATE

			if (is_null($pref) || $pref == 'SQ_DIV_DISABLE_DELETE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_DIV_DISABLE_DELETE']['name']));
			}
				$div_disable_delete = $this->pref_vars['SQ_DIV_DISABLE_DELETE']['default'];
				?>
				<table class="sq-backend-table" style="width: 100px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap">
						<?php
							echo translate('disable');
						?>
						</td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							combo_box('prefs[SQ_DIV_DISABLE_DELETE][default]', Array('yes' => 'Yes', 'no' => 'No'), FALSE, $div_disable_delete);
						} else {
							echo ucfirst($div_disable_delete);
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			$o->note(translate('div_disable_delete_note'));

			if (is_null($pref)) $o->closeField();

		}//end if show SQ_DIV_DISABLE_DELETE
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
				$this->pref_vars['SQ_DIV_DISABLE_CREATE']['default'] = $_POST['prefs']['SQ_DIV_DISABLE_CREATE']['default'];
			}
		}

		// Div DISABLE DELETE
		if (is_null($pref) || $pref == 'SQ_DIV_DISABLE_DELETE') {
			if (isset($_POST['prefs']['SQ_DIV_DISABLE_DELETE'])) {
				$this->pref_vars['SQ_DIV_DISABLE_DELETE']['default'] = $_POST['prefs']['SQ_DIV_DISABLE_DELETE']['default'];
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
				$merged_prefs[$var_name]['protected']	= ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
			}
		}

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>

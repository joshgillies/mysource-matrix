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
	 * Default paint for a field.
	 *
	 * @param object Backend_Outputter    &$o           Reference to the backend outputter.
	 * @param boolean                     $have_lock    FALSE means paint as read-only.
	 * @param string|NULL                 $all_prefs    List of all preferences to paint.
	 * @param string                      $pref         Preference to paint.
	 * @param callback                    $paintFn      Paint function
	 *
	 * @return boolean
	 */
	function paintField(Backend_Outputter $o, $have_lock, $all_prefs, $pref, $paintFn)
	{
	    $commit = FALSE;

	    if (is_null($all_prefs) || $all_prefs === $pref) {
            if (array_key_exists($pref, $this->pref_vars) === TRUE) {
                $pref_value = $this->pref_vars[$pref]['default'];
            }

            if (is_null($all_prefs)) {
                $o->openField($this->pref_vars[$pref]['name']);
            }

            // Paint the field.
            $commit = $paintFn($pref_value);

            if (array_key_exists('desc', $this->pref_vars[$pref]) === TRUE) {
                $o->note($this->pref_vars[$pref]['desc']);
            }

            if (is_null($all_prefs)) {
                $o->closeField();
            }

            $commit = $commit && $have_lock;
        }

	    return $commit;

	}//end paintField()


	/**
	 * Paint a simple text field.
	 *
	 * If $format is a callback, then that function should return what is to
	 * be echoed.
	 *
	 * @param object Backend_Outputter    &$o           Reference to the backend outputter.
	 * @param boolean                     $have_lock    FALSE means paint as read-only.
	 * @param string|NULL                 $all_prefs    List of all preferences to paint.
	 * @param string                      $pref         Preference to paint.
	 * @param array                       $width        Width of the text box.
	 * @param string|callback             $format       Formatting string (sprintf) or callback.
	 *
	 * @return boolean
	 */
	function paintTextField(Backend_Outputter $o, $have_lock, $all_prefs, $pref, $width, $height=1, $format='%s')
	{
		return $this->paintField($o, $have_lock, $all_prefs, $pref, function($pref_value) use ($have_lock, $pref, $width, $height, $format) {
		   if ($have_lock) {
                if ($height === 1) {
                    text_box('prefs['.$pref.']', $pref_value, $width);
				} else {
				    text_area('prefs['.$pref.']', $pref_value, $width, $height);
				}
            } else {
                if (is_callable($format)) {
                    echo $format($pref_value);
                } else {
                    echo sprintf($format, $pref_value);
                }
            }

			return $have_lock;
		});

	}//end paintTextField()


	/**
	 * Paint a Integer text field.
	 *
	 * If $format is a callback, then that function should return what is to
	 * be echoed.
	 *
	 * @param object Backend_Outputter    &$o           Reference to the backend outputter.
	 * @param boolean                     $have_lock    FALSE means paint as read-only.
	 * @param string|NULL                 $all_prefs    List of all preferences to paint.
	 * @param string                      $pref         Preference to paint.
	 * @param array                       $width        Width of the text box.
	 * @param string|callback             $format       Formatting string (sprintf) or callback.
	 *
	 * @return boolean
	 */
	function paintIntTextField(Backend_Outputter $o, $have_lock, $all_prefs, $pref, $width, $format='%s', $range_min=NULL, $range_max=NULL)
	{
		return $this->paintField($o, $have_lock, $all_prefs, $pref, function($pref_value) use ($have_lock, $pref, $width, $range_min, $range_max, $format) {
		    if ($have_lock) {
                ob_start();
                	int_text_box('prefs['.$pref.']', $pref_value, TRUE, $width, $range_min, $range_max, '', FALSE, FALSE, '', FALSE, TRUE);
                $text_box = ob_get_clean();
                if (is_callable($format)) {
                    echo $format($text_box);
                } else {
                    echo sprintf($format, $text_box);
                }
            } else {
                if (is_callable($format)) {
                    echo $format($pref_value);
                } else {
                    echo sprintf($format, $pref_value);
                }
            }

			return $have_lock;
		});

	}//end paintIntTextField()


	/**
	 * Paint a simple combo field.
	 *
	 * @param object Backend_Outputter    &$o           Reference to the backend outputter.
	 * @param boolean                     $have_lock    FALSE means paint as read-only.
	 * @param string|NULL                 $all_prefs    List of all preferences to paint.
	 * @param string                      $pref         Preference to paint.
	 * @param array                       $options      Array of options for the combo box.
	 *
	 * @return boolean
	 */
	function paintComboField(Backend_Outputter $o, $have_lock, $all_prefs, $pref, $options)
	{
        return $this->paintField($o, $have_lock, $all_prefs, $pref, function($pref_value) use ($have_lock, $pref, $options) {
		    if ($have_lock) {
                combo_box('prefs['.$pref.']', $options, FALSE, $pref_value);
            } else {
                echo $options[$pref_value];
            }

			return $have_lock;
		});

	}//end paintComboField()


	/**
	 * Paint a simple combo field.
	 *
	 * @param object Backend_Outputter    &$o           Reference to the backend outputter.
	 * @param boolean                     $have_lock    FALSE means paint as read-only.
	 * @param string|NULL                 $all_prefs    List of all preferences to paint.
	 * @param string                      $pref         Preference to paint.
	 * @param array                       $options      Array of options for the combo box.
	 *
	 * @return boolean
	 */
	function paintAssetMapField(Backend_Outputter $o, $have_lock, $all_prefs, $pref, $multiple=FALSE, $types=Array(), $callback=NULL)
	{
        return $this->paintField($o, $have_lock, $all_prefs, $pref, function($pref_value) use ($have_lock, $pref, $multiple, $types, $callback) {
		    if ($have_lock) {
                if ($multiple === TRUE) {
                    if (is_array($pref_value) === FALSE) {
                        $pref_value = Array();
                    }
                    multiple_asset_finder('prefs['.$pref.']', $pref_value, $types);
                } else {
                    asset_finder('prefs['.$pref.']', $pref_value, $types);
                }
            } else {
                if (is_callable($callback) === TRUE) {
                    echo $callback($pref_value);
                } else {
                    if ($multiple === TRUE) {
                        if (empty($pref_value) === TRUE) {
                            if (is_string($callback) === TRUE) {
                                echo $callback;
                            } else {
                                echo translate('No assets selected');
                            }
                        } else {
                            echo '<ul>';
                            foreach ($pref_value as $asset) {
                                echo '<li>'.get_asset_tag_line($asset).'</li>';
                            }
                            echo '</ul>';
                        }
                    } else {
                        if (empty($pref_value) === TRUE) {
                            if (is_string($callback) === TRUE) {
                                echo $callback;
                            } else {
                                echo translate('No asset selected');
                            }
                        } else {
                            echo get_asset_tag_line($pref_value);
                        }
                    }
                }
            }

			return $have_lock;
		});

	}//end paintAssetField()


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

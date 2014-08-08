#!/usr/bin/php
<?php
/**
 * matrixsqlclient.php - Interactive database terminal in PHP.
 * https://github.com/dansimau/matrixsqlclient
 *
 * Copyright 2011, Daniel Simmons <dan@dans.im>
 *
 * Licensed under the MIT license.
 * http://opensource.org/licenses/mit-license.php
 *
 * Contains Array to Text Table Generation Class, copyright 2009 Tony Landis.
 * Licensed under the BSD license.
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * 2011-10-28 (rev 113)
 *
 */
$rev = "version 1.01, r113";

define('UP', chr(27).chr(91).chr(65));
define('DOWN', chr(27).chr(91).chr(66));
define('RIGHT', chr(27).chr(91).chr(67));
define('LEFT', chr(27).chr(91).chr(68));

// Install signal handler (if possible)
if (function_exists("pcntl_signal")) {
	declare(ticks = 1);

	function sig_handler($signal)
	{
		global $matrixSqlTerminal;
		switch ($signal) {
			case SIGINT:
				// Tell SQL client to cancel what it's doing
				$matrixSqlTerminal->cancel();
				break;
			case SIGCONT:
				// Reset the terminal again when the process is unfrozen
				$matrixSqlTerminal->resetTerminal();
				break;
		}
	}
	pcntl_signal(SIGCONT, "sig_handler");
	pcntl_signal(SIGINT, "sig_handler");
}

error_reporting(E_ALL);
mb_internal_encoding("UTF-8");

// Run the terminal
try {
	$matrixSqlTerminal = new InteractiveSqlTerminal('MatrixDAL');
	$matrixSqlTerminal->connect((isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '');
	$matrixSqlTerminal->setOption("HISTFILE", "~/.matrixsqlclient_history");
	$matrixSqlTerminal->run();
}
catch (Exception $e) {
	// If there is some kind of problem, try to ensure the terminal is reset
	// for the user.
	`stty sane`;
	$matrixSqlTerminal->restoreTerminal();
}

/**
 * Terminal handling functions
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
class Terminal
{
	/**
	 * Moves the cursor left.
	 *
	 * @param integer $c the number of characters to move the cursor left
	 *
	 * @return void
	 */
	public static function left($c=1) {
		for ($i=0; $i<$c; $i++) echo chr(8);
	}
	
	/**
	 * Backspaces the text at the current position of the cursor
	 *
	 * @param integer $c the number of characters backspace
	 *
	 * @return void
	 */
	public static function backspace($c=1) {
		self::left($c);
		for ($i=0; $i<$c; $i++) echo ' ';
		self::left($c);
	}

	/**
	 * Returns the height and width of the terminal.
	 *
	 * @return array An array with two elements - number of rows and number of
	 *               columns.
	 */
	public function getTtySize()
	{
		return explode("\n", `printf "lines\ncols" | tput -S`);
	}

	/**
	 * Ouputs a bell character.
	 *
	 * @return void
	 */
	public static function bell()
	{
		echo chr(7);
	}

}

/**
 * SQL client - the main class.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
class InteractiveSqlTerminal
{
	/**
	 * @var $_db DbBackend object for the backend/database
	 */
	private $_db;

	/**
	 * @var $_historyFile Path to the file where command history will be saved
	 */
	private $_historyFile = '';

	/**
	 * @var $_tty_saved Stores stty string of saved terminal settings
	 */
	private $_tty_saved = 'sane';

	/**
	 * @var $_shell SimpleReadline object
	 */
	private $_shell;

	/**
	 * @var $_output_buffer line output buffer
	 */
	private $_output_buffer = array();

	/**
	 * @var $_options An array with a list of options and values
	 */
	private $_options = array(
		'HISTFILE' => '~/.phpsqlc_history',
		'HISTSIZE' => 500,
		'timing' => "off",
		'disable-completion' => "off",
		'rowlimit' => 500,
		'pager' => 'on',
	);

	/**
	 * @var $_cancel flag indicating whether the current query has been requested
	 *               to be cancelled.
	 */
	private $_cancel = false;

	/**
	 * Constructor - initialises Matrix DAL and attempts to connect to database
	 *
	 * @param string $backend name of backend plugin to use to connect
	 */
	public function __construct($backend)
	{
		$this->resetTerminal(true);

		// Instantiate database backend plugin
		$this->_db = new DbBackend($backend);

		// Instantiate/initialise stuff
		$this->_shell = new SimpleReadline();

		// Parse options; set autocomplete on/off, etc.
		$this->_parseOptions();
	}
	
	/**
	 * Destructor function - should restore terminal settings
	 */
	public function __destruct()
	{
		$this->_shell->writeHistory($this->_historyFile);
		$this->restoreTerminal();
	}

	/**
	 * Attempt to cancel a currently-running query.
	 */
	public function cancel() {
		echo "Cancel request\n";
		$this->_cancel = true;
	}

	/**
	 * Connects the db backend
	 *
	 * @param string $dsn connection string for database
	 *
	 * @return true on success or false on failure
	 */
	public function connect($dsn)
	{
		return $this->_db->connect($dsn);
	}

	/**
	 * Starts the main interactive terminal
	 *
	 * @return void
	 */
	public function run()
	{
		$this->_shell->readHistory($this->_historyFile);

		$prompt = '=# ';
		$sql = '';

		ob_start();
		echo "Welcome to matrixsqlclient";
		if (!empty($GLOBALS['rev'])) {
			echo " (" . $GLOBALS['rev'] . ")";
		}
		echo ", the interactive database terminal in PHP.";
		echo "\n\nYou are now connected.";
		echo "\nDatabase type: " . $this->_db->getDbType() . $this->_db->getDbVersion() . ".\n\n";
		ob_end_flush();
		
		while (1) {

			// Prompt for input
			$line = $this->_shell->readline($this->_db->getDbName() . $prompt);

			if ($line === "") {
				echo "\n";
				continue;
			}

			// Exits
			if ((mb_substr(trim($line), 0, 4) == 'exit') || (mb_substr(trim($line), 0, 4) == 'quit') || (mb_substr(trim($line), 0, 2) == '\q')) {
				echo "\n";
				exit;
			}
			if (mb_substr($line, mb_strlen($line)-1, mb_strlen($line)) === chr(4)) {
				echo "\q\n";
				exit;
			}

			// CTRL-C cancels any current query
			if (ord(mb_substr($line, mb_strlen($line)-1, mb_strlen($line))) === 3) {
				$sql = '';
				$line = '';
				$prompt = '=# ';
				echo "\n";
				continue;
			}

			if (mb_strlen($line) > 0) {
				// Add this command to the history
				$this->_shell->addHistory(strtr($line, "\n", " "));
			}

			if (mb_substr(trim($line), 0, 7) == "\\timing") {

				$this->setOption("timing", !$this->_getOptionValue("timing"));

				if ($this->_getOptionValue("timing")) {
					echo "\nTiming is on.";
				} else {
					echo "\nTiming is off.";
				}

				echo "\n";
				continue;
			}

			// "\set" command
			if (strlen(trim($sql)) === 0 && mb_substr(trim($line), 0, 4) == "\set") {

				$params = explode(" ", $line, 3);

				// "\set" with no options - show existing options/values
				if (count($params) === 1) {

					$options = $this->_getOptions();

					if (count($options) > 0) {

						foreach ($this->_getOptions() as $option => $value) {
							$value = ($value === true) ? "on" : $value;
							$value = ($value === false) ? "off" : $value;
							echo "\n" . $option . " = '" . $value . "'";
						}
					}

				// "set" a particular value
				} else {

					$params = array_pad($params, 3, "");
					$this->setOption($params[1], $params[2]);
					$this->_parseOptions();
				}

				echo "\n";
				continue;
			}

			$sql .= "\n" . $line;

			// If the SQL string is terminated with a semicolon, or the DB module wants
			// to accept it (eg. for a macro), then execute it
			if ($this->_db->matchesMacro($sql) || mb_strpos($sql, ';')) {

				echo "\n";

				$sql = trim($sql);

				try {
					// Run the SQL
					$this->restoreTerminal();
					$source_data = @$this->_db->execute($sql);
				}
				catch (Exception $e) {
					echo "\n" . $e->getMessage() . "\n";

					$this->resetTerminal(true);

					// Reset the prompt cause its a new query
					$prompt = '=# ';
					$sql = '';

					echo "\n";
					continue;
				}

				$this->resetTerminal(true);

				// If cancel request was triggered then just discard anything that might
				// come back
				if ($this->_cancel) {
					$source_data = null;
					$this->_cancel = false;

					echo "Cancelled\n";

					$prompt = '=# ';
					$sql = '';
					continue;
				}

				// If we get an array back, it's rows
				if (is_array($source_data)) {

					$rowlimit = (int)$this->_getOptionValue("rowlimit");

					if (count($source_data) > $rowlimit) {
						$this->_addToLinesBuffer(explode("\n", "\n\nWARNING: Number of rows returned exceeded rowlimit.\nOnly the first $rowlimit rows are being shown. Use \set rowlimit <num> to adjust.\n\n"));
						$source_data = array_slice($source_data, 0, $rowlimit);
					}

					// Only render the table if rows were returned
					if (!empty($source_data)) {

						// Render the table
						$table = new ArrayToTextTable($source_data);
						$table->showHeaders(true);

						$data = explode("\n", $table->render(true));
						array_pop($data);
						$this->_addToLinesBuffer($data);
					}

					// Build count summary (at end of table) and add to line buffer
					$count_str = "(" . count($source_data) . " row";
					if (count($source_data) !== 1) {
						$count_str .= "s";
					}
					$count_str .= ")";

					$this->_addToLinesBuffer(array($count_str, ""));

				// Assuming it's a string...
				} else {
					$this->_addToLinesBuffer(array($source_data));
				}

				if ($this->_getOptionValue("timing")) {
    				// Output amount of time this query took
					$this->_addToLinesBuffer(array("Time: " . $this->_db->getQueryExecutionTime() . " ms"));
    			}

    			// Output the data
    			$this->_printLines();
		
				// Reset the prompt cause its a new query
				$prompt = '=# ';
				$sql = '';
		
			} elseif (mb_strlen(trim($sql)) > 0) {
				// We're in the middle of some SQL, so modify the prompt slightly to show that
				// (like psql does)
				if ((substr_count($sql, "(") > substr_count($sql, ")"))) {
					$prompt = '(# ';
				} else {
					$prompt = '-# ';
				}
				echo "\n";
			}
		}
	}
	
	/**
	 * Restores the terminal to the previously saved state.
	 *
	 * @return void
	 */
	public function restoreTerminal()
	{
		system("stty '" . trim($this->_tty_saved) . "'");
	}

	/**
	 * Provides autocompletion for the given text.
	 *
	 * @param string $hint Current non-completed text string
	 *
	 * @return string Autocomplete matches
	 */
	public function autoComplete($hint)
	{
		$last_word = ltrim(mb_substr($hint, mb_strrpos($hint, ' ')));

		// Autocomplete table names after a FROM
		if (preg_match('/SELECT\s+.+\s+FROM\s+\w*$/i', $hint)) {
			$candidates = $this->_db->getTableNames();

		// Autocomplete table names after UPDATE
		} elseif (preg_match('/UPDATE\s+\w*$/i', $hint)) {
			$candidates = $this->_db->getTableNames();

		// Autocomplete table names at INSERT INTO
		} elseif (preg_match('/INSERT INTO\s+\w*$/i', $hint)) {
			$candidates = $this->_db->getTableNames();

		// Autocomplete column names after a WHERE
		} elseif (preg_match('/SELECT\s+.+\s+FROM\s+(.+?)\s+WHERE\s+\w*$/i', $hint, $table_name_search)) {
			$table_name = @$table_name_search[1];
			$candidates = $this->_db->getColumnNames($table_name);

		// Autocomplete column names at UPDATE..SET
		} elseif (preg_match('/UPDATE\s+(.+?)\s+SET\s+\w*$/i', $hint, $table_name_search)) {
			$table_name = @$table_name_search[1];
			$candidates = $this->_db->getColumnNames($table_name);
		}

		// Nothing to autocomplete
		if (empty($candidates)) {
			return array();
		}

		// Autocomplete has options, but user hasn't begun typing yet
		if ($last_word === "") {
		    return $candidates;
		}

		// Autocomplete has options, lets narrow it down based on what the user has
		// typed already.
		$matches = array();
		foreach ($candidates as $candidate) {
			if (mb_strpos($candidate, $last_word) === 0) {
				$matches[] = $candidate;
			}
		}

		return $matches;
	}

	/**
	 * Gets the terminal ready for our own use (switch it to raw mode).
	 *
	 * @param bool $save_existing whether to save the existing terminal settings for
	 *                            restoring later.
	 *
	 * @return void
	 */
	public function resetTerminal($save_existing=true)
	{
		// Save existing settings
		if ($save_existing) {
			$this->_tty_saved = `stty -g`;
		}

		// Reset terminal
		system("stty raw opost -ocrnl onlcr -onocr -onlret icrnl -inlcr -echo isig intr undef");
	}

	/**
	 * Prints the specified number of lines from the line buffer.
	 *
	 * @param integer $n number of lines to print, or 0 to print all lines, with
	 *                pagination (default)
	 *
	 * @return void
	 */
	private function _printLines($n=0)
	{

		$lines_printed = array();

		if ($n > 0) {

			// Print a specific number of lines
			$line_buffer_len = count($this->_output_buffer);
			for ($i=0; $i<$line_buffer_len && $i<$n; $i++) {
				$line = array_shift($this->_output_buffer);
				echo $line;
				$lines_printed[] = $line;
			}

			// Return the lines printed
			return $lines_printed;

		} else {

			// Get current terminal size
			$tty_size = $this->_getTtySize();

			if (!(bool)$this->_getOptionValue("pager") === true || count($this->_output_buffer) < $tty_size[0]) {

				// Print all lines, if it fits on the tty
				$this->_printLines(count($this->_output_buffer));

			} else {

				// Otherwise, let's paginate...

				// Print first chunk
				$last_lines = $this->_printLines($tty_size[0]-1);
				if ($last_lines[count($last_lines)-1][mb_strlen($last_lines[count($last_lines)-1])-1] != "\n") {
					echo "\n";
				}
				echo "\033[30;47m" . "--More--" . "\033[0m";

				// Print rest of the chunks
				while (1) {

					// Stop printing chunks if the line buffer is empty
					if (!count($this->_output_buffer) > 0) {
						// Backspace the "--More--"
						Terminal::backspace(8);
						break;
					}

					// Read user input
					$c = SimpleReadline::readKey();

					switch ($c) {

						// 'G' -- print rest of all the output
						case chr(71):
							Terminal::backspace(8);
							$this->_printLines(count($this->_output_buffer));
							break;

						// User wants more lines, one at a time
						case chr(10):

							// Backspace the "--More--"
							Terminal::backspace(8);

							$last_lines = $this->_printLines(1);
							if ($last_lines[count($last_lines)-1][mb_strlen($last_lines[count($last_lines)-1])-1] != "\n") {
								echo "\n";
							}
							echo "\033[30;47m" . "--More--" . "\033[0m";

							break;

						// Page down
						case chr(32):
						case chr(122):

							// Backspace the "--More--"
							Terminal::backspace(8);

							$last_lines = $this->_printLines($tty_size[0]-1);
							if ($last_lines[count($last_lines)-1][mb_strlen($last_lines[count($last_lines)-1])-1] != "\n") {
								echo "\n";
							}
							echo "\033[30;47m" . "--More--" . "\033[0m";

							break;

						// User wants to end output (ie. 'q', CTRL+C)
						case chr(3):
						case chr(113):

							// Backspace the "--More--"
							Terminal::backspace(8);

							// Clear line buffer
							$this->_clearLineBuffer();

							return;
							break;

						default:
							Terminal::bell();
							continue;
					}
				}
			}
		}
	}

	/**
	 * Adds data to the line buffer.
	 *
	 * @param array $data array of lines to add to the buffer
	 *
	 * @return void
	 */
	private function _addToLinesBuffer($data)
	{
		// Get current terminal size
		$tty_size = $this->_getTtySize();

		// Loop through data so we can split lines at terminal size
		for ($i=0; $i<count($data); $i++) {

			// Add newlines to the end of each proper line
			$data[$i] .= "\n";

			// Split line at terminal width and add to output
			foreach (str_split($data[$i], (int)$tty_size[1]) as $line) {
				$this->_output_buffer[] = $line;
			}
		}
	}

	/**
	 * Erases everything in the line buffer.
	 *
	 * @return void
	 */
	private function _clearLineBuffer()
	{
		$this->_output_buffer = array();
	}

	/**
	 * Set an sqlclient option.
	 *
	 * @param string $option name of the option
	 * @param mixed  $value  of the option
	 *
	 * @return void
	 */
	public function setOption($option, $value)
	{
		$this->_options[$option] = $value;
		$this->_parseOptions();
	}

	/**
	 * Get all the sqlclient options.
	 *
	 * @return array an array of all the options and their settings
	 */
	private function _getOptions()
	{
		return $this->_options;
	}

	/**
	 * Get an sqlclient option value.
	 *
	 * @param string $option the name of the option to get the current value of
	 *
	 * @return mixed the value of the specified option
	 */
	private function _getOptionValue($option)
	{
		$value = false;

		if (isset($this->_options[$option])) {

			$value = trim(strtolower($this->_options[$option]));

			switch ($value) {

				case "true":
				case "yes":
				case "on":
				case "1":
					$value = true;
					break;

				case "false":
				case "off":
				case "no":
				case "0":
					$value = false;
					break;
			}
		}

		return $value;
	}

	/**
	 * Returns the height and width of the terminal.
	 *
	 * @return array An array with two elements - number of rows and number of
	 *               columns.
	 */
	private function _getTtySize()
	{
		return explode("\n", `printf "lines\ncols" | tput -S`);
	}

	/**
	 * Performs one-time action things that need to be done when options are
	 * toggled on or off.
	 *
	 * @return void
	 */
	private function _parseOptions()
	{
		// Register autocomplete function
		if (!$this->_getOptionValue("disable-completion")) {
			$this->_shell->registerAutocompleteCallback(array($this, "autoComplete"));
		} else {
			$this->_shell->registerAutocompleteCallback(null);
		}

		// Set maximum history size
		$this->_shell->setHistorySize($this->_getOptionValue("HISTSIZE"));

		// Expand out tilde (~) in history filename
		if (strpos($this->_getOptionValue("HISTFILE"), "~") !== false) {
			if (isset($_ENV['HOME'])) {
				$this->_historyFile = str_replace("~", $_ENV['HOME'], $this->_getOptionValue("HISTFILE"));
			} else {
				$this->_historyFile = str_replace("~", "/tmp", $this->_getOptionValue("HISTFILE"));
			}
		} else {
			$this->_historyFile = $this->_getOptionValue("HISTFILE");
		}
	}
}

/**
 * Alternative readline library.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */

function sortArrayByLength($a,$b)
{
	return count($a)-count($b);
}


//system("stty raw opost -ocrnl onlcr -onocr -onlret icrnl -inlcr -echo isig intr undef");

class SimpleReadline
{
	/**
	 * @var Stores the command line history.
	 */
	private $_history = array();
	
	/**
	 * @var Stores a working copy the command line history.
	 */
	private $_history_tmp = array();
	
	/**
	 * @var Stores the current position in the command line history.
	 */
	private $_history_position = -1;
	
	/**
	 * @var $history_stage HistoryStorage class that saves command history to a file
	 */
	private $_history_storage = null;

	/**
	 * @var Stores the data of the line the user is currently typing.
	 */
	private $_buffer = '';
	
	/**
	 * @var Stores current cursor position
	 */
	private $_buffer_position = 0;

	/**
	 * @var Name of the user-defined function that is called for autocompletion
	 */
	private $_autocomplete_callback = null;

	/**
	 * @var Prompt prefix
	 */
	private $_prompt = null;

	/**
	 * @var number of times TAB has been pressed since last autocomplete
	 */
	private $_autocompleteTabPressCount = 0;

	/**
	 * Adds a line to the command line history.
	 *
	 * @param string $line Line to be added in the history.
	 *
	 * @return bool Returns true on success or false on failure.
	 */
	public function addHistory($line)
	{
		return ($this->_history[] = trim($line));
	}

	/**
	 * Reads a command history from a file.
	 *
	 * @param string $filename Path to the filename containing the command history.
	 *
	 * @return boolean true on success or false on failure
	 */
	public function readHistory($filename)
	{
		$this->_history_storage = null;
		$this->_history_storage = new HistoryStorage($filename);

		if ($this->_history_storage->load()) {
			$this->_history = $this->_history_storage->getData();
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Sets the maximum number of history items that will be saved to file.
	 *
	 * @param integer $c Maximum number of history items to save to file.
	 *
	 * @return true on success or false on failure
	 */
	public function setHistorySize($c)
	{
		if (is_integer($c)) {
			$this->_history_storage->setMaxSize($c);
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Reads a single line from the user.
	 *
	 * @param string $prompt You may specify a string with which to prompt the user.
	 *
	 * @return Returns a single string from the user.
	 */
	public function readline($prompt=null)
	{
		$line = null;

		$this->_reset();
	
		// Output prompt
		if ($prompt !== null) {
			$this->_prompt = $prompt;
			echo $prompt;
		}
		
		while (1) {
		
			$c = self::readKey();
		
			switch ($c) {

				// Unrecognised character
				case null:
					Terminal::bell();
					break;

				// TAB
				case chr(9):

					// If autocompletion is registered, then do it
					if ($this->_autocomplete_callback !== null) {

						$autocomplete_text = $this->_doAutocomplete($this->_buffer);

						if (!empty($autocomplete_text)) {
							$this->_insert($autocomplete_text);
						} else {
							Terminal::bell();
						}

					// Otherwise, TAB will insert spaces
					} else {
						$this->_insert("        ");
					}

					break;

				// CTRL-A (Home) - move the cursor all the way to the left
				case chr(1):
					$this->_cursorLeft($this->_buffer_position);
					break;

				// CTRL-E (End) - move cursor all the way to the end
				case chr(5):
					$this->_cursorRight(mb_strlen($this->_buffer) - $this->_buffer_position);
					break;
				
				// Line-delete - backspace from current position to beginning of line
				case chr(21):
					$this->_backspace($this->_buffer_position);
					break;

				// Word-delete (CTRL-W)
				case chr(23):

					// Get previous word position
					$prev_word_pos = $this->_buffer_position-$this->_getPreviousWordPos();

					// Delete word, unless we're at the start of the line, then bell
					if ($prev_word_pos > 0) {
						$this->_backspace($this->_buffer_position-$this->_getPreviousWordPos());
					} else {
						Terminal::bell();
					}

					break;

				// CTRL-LEFT
				case chr(27) . chr(91) . chr(53) . chr(68):
					$this->_cursorLeft($this->_buffer_position-$this->_getPreviousWordPos());
					break;

				// CTRL-RIGHT
				case chr(27) . chr(91) . chr(53) . chr(67):
					$this->_cursorRight($this->_getNextWordPos()-$this->_buffer_position);
					break;

				// CTRL-C
				case chr(3):
						$line = $this->_buffer . $c;
						break;

				// CTRL-D
				case chr(4):
				
					// Return current line immediately on CTRL-D
					if (mb_strlen($this->_buffer) === 0) {
						$line = $this->_buffer . $c;
					} else {
						Terminal::bell();
					}
					break;

				case UP:
					// Move backwards in the history
					if (!$this->_historyMovePosition(-1)) {
						Terminal::bell();
					}
					break;

				case DOWN:
					// Move forward in the history
					if (!$this->_historyMovePosition(1)) {
						Terminal::bell();
					}
					break;

				case LEFT:
					// Move left, or beep if we're already at the beginning
					if (!$this->_cursorLeft()) {
						Terminal::bell();
					}
					break;

				case RIGHT:
					// Move right, or beep if we're already at the end
					if (!$this->_cursorRight()) {
						Terminal::bell();
					}
					break;

				// Backspace key
				case chr(8):
				// Delete
				case chr(127):

					if (!$this->_backspace()) {
						Terminal::bell();
					}
					break;

				// Enter key
				case chr(10):

					// Set the $line variable so we return below
					$line = $this->_buffer;
					break;

				// Normal character key
				default:
				
					// Ignore unknown control characters
					if (ord($c[0]) === 27) {
						Terminal::bell();
						continue;
					}

					// Insert this character into the buffer and move on
					$this->_insert($c);
			}

			// If line has been set, we're ready to do something with this command
			if ($line !== null) {
			
				// Firstly check for internal commands
				if ($this->_runInternalCommand(trim($line))) {

					// It it was an internal command, don't return, just reset and pretend
					// nothing happened...
					$this->addHistory($line);
					$line = null;
					$this->_reset();
				}

				// Remove temp history item
				array_pop($this->_history_tmp);

				return $line;
			}
		}
	}

	/**
	 * Returns data from a keypress. This will either be a single character, or a set of control
	 * characters.
	 *
	 * @return Returns a string containing a character or set of control characters.
	 */
	public static function readKey()
	{
		$buffer = null;
		$key = null;

		while (1) {

			$c = fgetc(STDIN);

			$buffer .= $c;

			// Handle control characters
			if (ord($buffer[0]) === 27) {

				if ((strlen($buffer) === 1) && (ord($c) === 27)) {
					continue;
				} elseif ((strlen($buffer) === 2) && (ord($c) === 91)) {
					continue;
				} elseif (strlen($buffer) === 3 && ord($c) >= 30 && ord($c) <= 57) {
					continue;
				} else {
					return $buffer;
				}
			}

			// Handle other characters and multibyte characters
			if (self::_isValidChar($buffer)) {
				return $buffer;
			}

			// Safeguard in case isValidChar() fails - UTF-8 characters will never be
			// more than 4 bytes. Something's gone wrong, so return null
			if (strlen($buffer) > 4) {
				return null;
			}
		}
	}

	/**
	 * Registers the function that will be called when TAB is pressed on the prompt:
	 * function takes one parameter, the "hint", and returns the extra text to be
	 * added to the current line
	 *
	 * @param callback $f callback the function to call for autocompletion
	 *
	 * @return void
	 */
	public function registerAutocompleteCallback($f)
	{
		$this->_autocomplete_callback = $f;
	}

	/**
	 * Writes the command history to a file.
	 *
	 * @param string $filename Path to the saved file.
	 *
	 * @return boolean true on success or false on failure
	 */
	public function writeHistory($filename)
	{
		if (get_class($this->_history_storage) !== "HistoryStorage") {
			return false;
		}

		$this->_history_storage->setData($this->_history);

		if ($this->_history_storage->save() !== false) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Backspaces characters.
	 *
	 * @param int $n The number of characters to backspace.
	 *
	 * @return true on success, false on failure
	 */
	private function _backspace($n=1)
	{
		if ($this->_buffer_position < $n) {
			// We can't backspace this far
			return false;
		}

		ob_start();

		for ($i=0; $i<$n; $i++) {
			if ($this->_buffer_position < mb_strlen($this->_buffer)) {
	
				$head = mb_substr($this->_buffer, 0, $this->_buffer_position);
				$tail = mb_substr($this->_buffer, $this->_buffer_position, mb_strlen($this->_buffer));
				
				Terminal::backspace();
				echo $tail . ' ';
				Terminal::left(mb_strlen($tail)+1);
				
				// Update buffer
				$this->_buffer = mb_substr($head, 0, mb_strlen($head)-1) . $tail;

			} else {
	
				// Just backspace one char
				$this->_buffer = mb_substr($this->_buffer, 0, mb_strlen($this->_buffer)-1);
				Terminal::backspace();
			}
	
			$this->_buffer_position--;
		}

		ob_end_flush();

		return true;
	}

	/**
	 * Move up or down in the history.
	 *
	 * @param integer $n specifying how many places to move up/down in the history
	 *
	 * @return true on success, false on failure
	 */
	private function _historyMovePosition($n)
	{
		// Check we can actually move this far
		if (!array_key_exists($this->_history_position + $n, $this->_history_tmp)) {
		
			return false;

		} else {

			ob_start();

			// Clear current line
			$this->_cursorRight(mb_strlen($this->_buffer) - $this->_buffer_position);
			$this->_backspace($this->_buffer_position);

			// Move forward/back n number of positions
			$this->_history_position = $this->_history_position + $n;
	
			// Print history item and set buffer
			echo $this->_history_tmp[$this->_history_position];
			$this->_buffer = $this->_history_tmp[$this->_history_position];
			$this->_buffer_position = mb_strlen($this->_buffer);

			ob_end_flush();

			return true;
		}

	}
	
	/**
	 * Inserts the specified text/character into the current position in the buffer
	 *
	 * @param string $c the character or string to insert
	 *
	 * @return void
	 */
	private function _insert($c)
	{
		// If the cursor is in the middle of the line...
		if ($this->_buffer_position < mb_strlen($this->_buffer)) {

			$head = mb_substr($this->_buffer, 0, $this->_buffer_position);
			$tail = mb_substr($this->_buffer, $this->_buffer_position, mb_strlen($this->_buffer));

			ob_start();
			echo $c . $tail;
			Terminal::left(mb_strlen($tail));
			$this->_buffer = $head . $c . $tail;
			ob_end_flush();

		} else {

			// Otherwise just append/echo it don't worry about the other stuff
			$this->_buffer .= $c;
			echo $c;	// User's terminal must take care of multibyte characters
		}

		$this->_buffer_position = $this->_buffer_position + mb_strlen($c);
		$this->_history_tmp[$this->_history_position] = $this->_buffer;
	}

	/**
	 * Checks a sequence of bytes and returns whether or not that sequence form a
	 * valid character under the current encoding.
	 *
	 * @param string $sequence string of bytes to check
	 *
	 * @return boolean true if string is valid, false if not
	 */
	private static function _isValidChar($sequence)
	{

		$encoding = mb_internal_encoding();

		// Check for bad byte stream
		if (mb_check_encoding($sequence) === false) {
			return false;
		}

		// Check for bad byte sequence
		$fs = $encoding == 'UTF-8' ? 'UTF-32' : $encoding;
		$ts = $encoding == 'UTF-32' ? 'UTF-8' : $encoding;

		if ($sequence !== mb_convert_encoding(mb_convert_encoding($sequence, $fs, $ts), $ts, $fs)) {
			return false;
		}

		return true;
	}
	
	/**
	 * Moves the cursor left.
	 *
	 * @param integer $n The number of characters left to move the cursor.
	 *
	 * @return boolean true on success or false on failure
	 */
	private function _cursorLeft($n=1)
	{
		// Move cursor left if we can
		if ($this->_buffer_position > 0) {

			$this->_buffer_position = $this->_buffer_position - $n;
			Terminal::left($n);

			return true;

		} else {
			return false;
		}
	}
	
	/**
	 * Move cursor to the right.
	 *
	 * @param integer $n Number of characters to the right to move the cursor.
	 *
	 * @return boolean Whether or not the cursor was able to be moved to the right
	 */
	private function _cursorRight($n=1)
	{
		if ($this->_buffer_position < mb_strlen($this->_buffer)) {

			for ($i=0; $i<$n; $i++) {
				echo mb_substr($this->_buffer, $this->_buffer_position, 1);
				$this->_buffer_position++;
			}

			return true;

		} else {

			// Return false if the cursor is already at the end of the line
			return false;
		}
	}

	/**
	 * Calls user-defined autocomplete function to complete the current string.
	 *
	 * @param string $hint Line of typed text to pass to the callback function.
	 *
	 * @return mixed returns the partial text to complete, or false if nothing
	 */
	private function _doAutocomplete($hint)
	{
		if ($this->_autocomplete_callback === null) {
			return false;
		}

		$candidates = call_user_func($this->_autocomplete_callback, $hint);

		if (empty($candidates)) {
			return false;
		}

		$last_word = mb_substr($hint, mb_strrpos($hint, ' ')+1);

		/* If the last word is nothing '', then it means the user hasn't started off
		   the autocomplete (given a hint) at all. We don't do inline autocomplete in
		   this case. */
		if ($last_word === '') {
			$this->_showAutoCompleteOptions($candidates);
			return false;
		}

		/* Otherwise, the user has started typing a word, and we want to autocomplete
		   it in-line as much as possible before showing possible options. */
		$matches = array();
		foreach ($candidates as $match) {
			if (mb_strpos($match, $last_word) === 0) {
				$matches[] = mb_substr($match, mb_strlen($last_word));
			}
		}

		if (empty($matches)) {
			return false;
		}

		// If there's only one match, return it, along with a space on the end
		if (count($matches) === 1) {
			return $matches[0] . " ";
		}

		// Otherwise, let's complete as many common letters as we can...

		$finalAutocompleteString = '';

		// Explode each character of each match into it's own array
		$candidate_map = array();
		foreach ($matches as $match) {
			$candidate_map[] = preg_split('/(?<!^)(?!$)/u', $match); // preg_split here for multibyte chars
		}

		// Sort matches by length, shortest first
		usort($candidate_map, 'sortArrayByLength');

		for ($i=0; $i<count($candidate_map[0]); $i++) {	// "Best match" can't be longer than shortest candidate

			$chars = array();

			// Get all the letters at position $i from all candidates
			foreach ($candidate_map as &$candidate) {
				$chars[] = $candidate[$i];
			}

			// Check if they are all the same letter
			$chars_uniq = array_unique($chars);
			if (count($chars_uniq) === 1) {
				$finalAutocompleteString .= $chars_uniq[0];
			}
		}

		if ($finalAutocompleteString === '') {
			$this->_showAutoCompleteOptions($candidates);
		}

		return $finalAutocompleteString;
	}

	/**
	 * Returns the buffer position of the previous word, based on current buffer position.
	 *
	 * @return integer the position of the first character of the previous word
	 */
	private function _getPreviousWordPos()
	{
		$temp_str = mb_substr($this->_buffer, 0, $this->_buffer_position);

		// Remove trailing spaces on the end
		$temp_str = rtrim($temp_str);

		// Get first reverse matching space
		if (mb_strlen($temp_str) === 0) {
			return 0;
		}
		$prev_word_pos = mb_strrpos($temp_str, ' ');

		// Add one, which is the beginning of the previous word (unless we're at the beginning of the line)
		if ($prev_word_pos > 0) {
			$prev_word_pos++;
		}

		return $prev_word_pos;
	}

	/**
	 * Returns the buffer position of the next word, based on current buffer position.
	 *
	 * @return integer the position of the first character of the next word
	 */
	private function _getNextWordPos()
	{
		$temp_str = mb_substr($this->_buffer, $this->_buffer_position, mb_strlen($this->_buffer));

		// Store length, so we can calculate how many spaces are trimmed in the next step
		$temp_str_len = mb_strlen($temp_str);

		// Trim spaces from the beginning
		$temp_str = ltrim($temp_str);

		// Trimmed spaces
		$trimmed_spaces = $temp_str_len - mb_strlen($temp_str);

		// Get first matching space
		$next_word_pos = mb_strpos($temp_str, ' ');

		// If there is no matching space, we're at the end of the string
		if ($next_word_pos === false) {
			$next_word_pos = mb_strlen($this->_buffer);
		} else {
			$next_word_pos = $this->_buffer_position + $trimmed_spaces + $next_word_pos;
		}

		return $next_word_pos;
	}

	/**
	 * Resets buffer information and position.
	 *
	 * @return void
	 */
	private function _reset()
	{
		// Reset buffer
		$this->_buffer = '';
		$this->_buffer_position = 0;

		// Reset working history
		$this->_history_tmp = $this->_history;
		$this->_history_tmp[] = '';
		$this->_history_position = count($this->_history);
	}
	
	/**
	 * Parses the given string and runs any internal commands.
	 *
	 * @param string $command the input string
	 *
	 * @return boolean whether an internal command matched and was run
	 */
	private function _runInternalCommand($command)
	{
		// history command
		if (mb_substr($command, 0, 5) === "\hist") {

			echo "\n\n";

			// Print history
			for ($i=0; $i<count($this->_history); $i++) {
				$p = strlen((string)count($this->_history)) + 1;
				printf("%" . $p . "s  %s\n", $i+1, $this->_history[$i]);
			}

			return true;
		} else {
			return false;
		}
	}

	/**
	 * Outputs a visual list of the autocomplete candidates.
	 *
	 * @param array $options an array of the candidates
	 *
	 * @return void
	 */
	private function _showAutoCompleteOptions($options)
	{
		// TAB must be pressed twice to show autocomplete options
		if (!$this->_autocompleteTabPressCount > 0) {
			$this->_autocompleteTabPressCount++;
			return;
		} else {
			$this->_autocompleteTabPressCount = 0;
		}

		$optionMaxChars = 0;

		// Get length of the longest match (for spacing)
		foreach ($options as $option) {
			if (mb_strlen($option)+2 > $optionMaxChars) {
				$optionMaxChars = mb_strlen($option) + 2; // +2 spaces to pad with
			}
		}

		// Get tty width
		$ttySize = Terminal::getTtySize();
		$ttyChars = $ttySize[1];

		// Calculate number of lines required
		$linesRequired = ceil((count($options)*$optionMaxChars) / $ttyChars);

		// Calculate number of items per line
		$itemsPerLine = floor($ttyChars / $optionMaxChars);

		for ($i=0; $i < count($options); $i++) {
			if ($i % $itemsPerLine === 0) {
				echo "\n";
			}

			printf("%-" . $optionMaxChars . "s", $options[$i]);
		}
		echo "\n";
		echo $this->_prompt . $this->_buffer;
	}
}

/**
 * Array to Text Table Generation Class
 *
 * @author Tony Landis <tony@tonylandis.com>
 * @link http://www.tonylandis.com/
 * @copyright Copyright (C) 2006-2009 Tony Landis
 * @license http://www.opensource.org/licenses/bsd-license.php
 */
class ArrayToTextTable
{
    /** 
     * @var array The array for processing
     */
    private $rows;

    /** 
     * @var int The column width settings
     */
    private $cs = array();

    /**
     * @var int The Row lines settings
     */
    private $rs = array();

    /**
     * @var int The Column index of keys
     */
    private $keys = array();

    /**
     * @var int Max Column Height (returns)
     */
    private $mH = 2;

    /**
     * @var int Max Row Width (chars)
     */
    private $mW = 10000;

    private $head  = false;
    private $pcen  = "+";
    private $prow  = "-";
    private $pcol  = "|";
    
    
    /** Prepare array into textual format
     *
     * @param array $rows The input array
     * @param bool $head Show heading
     * @param int $maxWidth Max Column Height (returns)
     * @param int $maxHeight Max Row Width (chars)
     */
    public function ArrayToTextTable($rows)
    {
        $this->rows =& $rows;
        $this->cs=array();
        $this->rs=array();
 
        if(!$xc = count($this->rows)) return false; 
        $this->keys = array_keys($this->rows[0]);
        $columns = count($this->keys);
        
        for($x=0; $x<$xc; $x++)
            for($y=0; $y<$columns; $y++)    
                $this->setMax($x, $y, $this->rows[$x][$this->keys[$y]]);
    }
    
    /**
     * Show the headers using the key values of the array for the titles
     * 
     * @param bool $bool
     */
    public function showHeaders($bool)
    {
       if($bool) $this->setHeading(); 
    } 
    
    /**
     * Set the maximum width (number of characters) per column before truncating
     * 
     * @param int $maxWidth
     */
    public function setMaxWidth($maxWidth)
    {
        $this->mW = (int) $maxWidth;
    }
    
    /**
     * Set the maximum height (number of lines) per row before truncating
     * 
     * @param int $maxHeight
     */
    public function setMaxHeight($maxHeight)
    {
        $this->mH = (int) $maxHeight;
    }
    
    /**
     * Prints the data to a text table
     *
     * @param bool $return Set to 'true' to return text rather than printing
     * @return mixed
     */
    public function render($return=false)
    {
        if($return) ob_start(); 
  
//        $this->printLine();
        $this->printHeading();
        
        $rc = count($this->rows);
        for($i=0; $i<$rc; $i++) $this->printRow($i);
        
        if($return) {
            $contents = ob_get_contents();
            ob_end_clean();
            return $contents;
        }
    }

    private function setHeading()
    {
        $data = array();  
        foreach($this->keys as $colKey => $value)
        { 
            $this->setMax(false, $colKey, $value);
            $data[$colKey] = $value;
        }
        if(!is_array($data)) return false;
        $this->head = $data;
    }

    private function printLine($nl=true)
    {
//        print ' ';
		$i = 0;
        foreach($this->cs as $key => $val) {
            print $this->prow .
                str_pad('', $val, $this->prow, STR_PAD_RIGHT) .
                $this->prow;
                
				if ($i < count($this->cs)-1) {
	                print $this->pcen;
	            } else {
	            	print ' ';
	            }
	        	$i++;
	    }
        if($nl) print "\n";
    }

    private function printHeading()
    {
        if(!is_array($this->head)) return false;

//		print ' ';
		$i = 0;
        foreach($this->cs as $key => $val) {
            print ' '.
                str_pad($this->head[$key], $val, ' ', STR_PAD_BOTH) .
                ' ';
			if ($i < count($this->cs)-1) {
                print $this->pcol;
            } else {
            	print ' ';
            }
        	$i++;
        }

        print "\n";
        $this->printLine();
    }

    private function printRow($rowKey)
    {
        // loop through each line
        for($line=1; $line <= $this->rs[$rowKey]; $line++)
        {
//            print ' ';
            for($colKey=0; $colKey < count($this->keys); $colKey++)
            { 
                print " ";
                print str_pad(substr($this->rows[$rowKey][$this->keys[$colKey]], ($this->mW * ($line-1)), $this->mW), $this->cs[$colKey], ' ', STR_PAD_RIGHT);
				if ($colKey < count($this->keys)-1) {
	                print " " . $this->pcol;
	            } else {
	                print "  ";
				}
            }  
            print "\n";
        }
    }

    private function setMax($rowKey, $colKey, &$colVal)
    { 
        $w = mb_strlen($colVal);
        $h = 1;
        if($w > $this->mW)
        {
            $h = ceil($w % $this->mW);
            if($h > $this->mH) $h=$this->mH;
            $w = $this->mW;
        }
 
        if(!isset($this->cs[$colKey]) || $this->cs[$colKey] < $w)
            $this->cs[$colKey] = $w;

        if($rowKey !== false && (!isset($this->rs[$rowKey]) || $this->rs[$rowKey] < $h))
            $this->rs[$rowKey] = $h;
    }
}

/**
 * Stores an array in memory, and reads/writes that array as lines in a file.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
class HistoryStorage
{
	/**
	 * @var Path and filename of the file on disk to save the data
	 */
	private $_file = '';
	
	/**
	 * @var Array of the data
	 */
	private $_data = array();
	
	/**
	 * @var Boolean whether the data in the memory should be saved to file on
	 *              destruction
	 */
	private $_autosave = false;

	/**
	 * @var integer the maximum number of items that will be saved to file
	 */
	private $_maxsize = 500;

	/**
	 * Constructor
	 *
	 * @param string  $file     path and filename where history should be saved
	 * @param boolean $autosave whether to save history items to file on destruct
	 */
	function __construct($file='', $autosave=true)
	{
		$this->_file = $file;
		$this->_autosave = $autosave;
	}

	/**
	 * Destructor - writes data to file if autosave flag is true
	 */
	function __destruct()
	{
		if ($this->_autosave) {
			$this->save();
		}
	}

	/**
	 * Reads lines from the file into memory.
	 *
	 * @return mixed the data from the file, or false if the file couldn't be read
	 */
	function load()
	{
		$data = @file($this->_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

		if (is_array($data) === true) {
			$this->_data = $data;
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Saves contents of memory into file.
	 *
	 * @return mixed number of bytes that were written to the file, or false on
	                 failure.
	 */
	function save()
	{
		while (count($this->_data) > $this->_maxsize) {
			array_shift($this->_data);
		}

		return @file_put_contents($this->_file, implode("\n", $this->_data));
	}

	/**
	 * Returns an array of the data stored in memory.
	 *
	 * @return array get all data stored in memory
	 */
	function getData()
	{
		return $this->_data;
	}

	/**
	 * Updates the array stored in the memory.
	 *
	 * @param array $data the data to store
	 *
	 * @return mixed void or false if supplied data is not an array
	 */
	function setData($data)
	{
		if (is_array($data)) {
			$this->_data = $data;
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Sets the maximum number of lines that will be saved to file.
	 *
	 * @param integer $n number of lines
	 *
	 * @return void
	 */
	function setMaxSize($n)
	{
		$this->_maxsize = (int)$n;
	}

	/**
	 * Shows the the maximum number of lines that will be saved to file as per the
	 * current configuration.
	 *
	 * @return integer the current max number of lines that will be saved
	 */
	function getMaxSize()
	{
		return $this->_maxsize;
	}
}

/**
 * DbBackend - wrapper for backend plugins.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
class DbBackend
{
	/**
	 * @var $_executionTime Used to measure the time a query takes to run.
	 */
	private $_executionTime;

	/**
	 * @var $backend instantiated plugin class
	 */
	private $_backend;

	/**
	 * Constructor
	 *
	 * @param string $pluginName name of the db backend plugin to use
	 */
	public function __construct($pluginName)
	{
		$backend = null;
		$pluginName = 'DbBackend_' . $pluginName;

		if (class_exists($pluginName)) {
			$backend = new $pluginName;
		}

		if (is_null($backend) || !get_parent_class($pluginName) == 'DbBackendPlugin') {
			echo("Cannot find valid DbBackendPlugin class \"" . $pluginName . "\".");
			exit(20);
		}

		$this->_backend = $backend;
	}

	/**
	 * Calls plugin to disconnect and reconnect to specified backend.
	 *
	 * @param string $dsn plugin's backend to connect to
	 *
	 * @return bool whether the connection was successful
	 */
	public function connect($dsn)
	{
		$this->disconnect();
		return $this->_backend->connect($dsn);
	}

	/**
	 * Calls plugin to return a friendly name/identifier for the connected backend.
	 *
	 * @return string friendly name/identifier for the currently connected backend
	 */
	public function getDbName()
	{
		return $this->_backend->getDbName();
	}

	/**
	 * Calls plugin to return the database type.
	 *
	 * @return string friendly name/identifier for the database backend
	 */
	public function getDbType()
	{
		return $this->_backend->getDbType();
	}

	/**
	 * Calls plugin to return a version identifier for the database backend
	 *
	 * @return string friendly version identifier for the database backend
	 */
	public function getDbVersion()
	{
		return $this->_backend->getDbVersion();
	}

	/**
	 * Calls plugin to disconnect from backend.
	 *
	 * @return bool if the disconnect was a success
	 */
	public function disconnect()
	{
		return $this->_backend->disconnect();
	}

	/**
	 * Calls plugin to execute specified query.
	 *
	 * @param string $sql the SQL to run
	 *
	 * @return mixed data array of results, or false if the query was invalid
	 */
	public function execute($sql)
	{
		$query_start_time = microtime(true);
		$result = $this->_backend->execute($sql);
		$query_end_time = microtime(true);

		$this->_executionTime = $query_end_time - $query_start_time;

		return $result;
	}

	/**
	 * Returns the execution time of the last query.
	 *
	 * @return float the time the query took to execute, in ms, to 3 decimal places
	 */
	public function getQueryExecutionTime()
	{
		return round($this->_executionTime * 1000, 3);
	}

	/**
	 * Call plugin to get a list of the available tables on the current database.
	 *
	 * @return array List of table names.
	 */
	public function getTableNames()
	{
		return $this->_backend->getTableNames();
	}

	/**
	 * Call plugin to get a list of the columns on the specified table.
	 *
	 * @param string $table Name of the table.
	 *
	 * @return array List of table names.
	 */
	public function getColumnNames($table)
	{
		return $this->_backend->getColumnNames($table);
	}

	/**
	 * Checks to see if the current line matches an internal command/macro.
	 *
	 * @param string $s Command
	 *
	 * @return boolean true if yes or false if not
	 */
	public function matchesMacro($s)
	{
		return $this->_backend->matchesMacro($s);
	}
}

/**
 * Abstract class for DB backend plugins.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
abstract class DbBackendPlugin
{
	/**
	 * Connects to the host/database.
	 *
	 * @param string $conn_string Connection string/DSN for connecting to the
	 *                            database.
	 *
	 * @return boolean true on success, false on failure
	 */
	abstract public function connect($conn_string);

	/**
	 * Disconnect from the database/host.
	 *
	 * @return boolean true on success, false on failure
	 */
	abstract public function disconnect();

	/**
	 * Execute the specified SQL/commands on the database.
	 *
	 * @param string $sql The SQL/command to send to the database.
	 *
	 * @return mixed string or array of returned data, or false on failure
	 */
	abstract public function execute($sql);

	/**
	 * Get the name of the current database.
	 *
	 * @return string Name of the database.
	 */
	abstract public function getDbName();

	/**
	 * Get a description of the database/backend type.
	 *
	 * @return string Name of the database system.
	 */
	abstract public function getDbType();

	/**
	 * Get the version of the database/backend type.
	 *
	 * @return string Version of the database system.
	 */
	abstract public function getDbVersion();

	/**
	 * Get a list of the available tables on the current database. Used for
	 * autocomplete.
	 *
	 * @return array List of table names.
	 */
	abstract public function getTableNames();

	/**
	 * Get a list of the available columns on the specified table. Used for
	 * autocomplete.
	 *
	 * @param string $table Name of the table
	 *
	 * @return array List of column names
	 */
	abstract public function getColumnNames($table);

	/**
	 * Checks whether the specified command is a supported or valid macro.
	 *
	 * @param string $s Command
	 *
	 * @return boolean true if yes or false if not
	 */
	abstract public function matchesMacro($s);
}

/**
 * MatrixDAL (Squiz Matrix) backend for DbBackend.
 *
 * @author    Daniel Simmons <dan@dans.im>
 * @link      https://github.com/dansimau/matrixsqlclient
 * @copyright 2010 Daniel Simmons
 * @license   http://www.opensource.org/licenses/mit-license.php
 */
class DbBackend_MatrixDAL extends DbBackendPlugin
{
	/**
	 * @var $_dsn DSN to connect to the database
	 */
	private $_dsn = '';

	/**
	 * @var $_db_type Database type: either 'oci' or 'pgsql'.
	 */
	private $_db_type = '';

	/**
	 * @var $_macros Stores an array of macros (shorthand commands).
	 */
	private $_macros = array();

	/**
	 * Constructor.
	 */
	public function __construct()
	{
		// Define macros
		$this->_macros = array(

			"pgsql" => array(

				"\dt" => "
					SELECT n.nspname as \"Schema\",
					  c.relname as \"Name\",
					  CASE c.relkind WHEN 'r' THEN 'table' WHEN 'v' THEN 'view' WHEN 'i' THEN 'index' WHEN 'S' THEN 'sequence' WHEN 's' THEN 'special' END as \"Type\",
					  r.rolname as \"Owner\"
					FROM pg_catalog.pg_class c
					     JOIN pg_catalog.pg_roles r ON r.oid = c.relowner
					     LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
					WHERE c.relkind IN ('r','')
					      AND n.nspname NOT IN ('pg_catalog', 'pg_toast')
					      AND pg_catalog.pg_table_is_visible(c.oid)
					ORDER BY 1,2;",
			),
			
			"oci" => array(
				"\dt" => "SELECT * FROM tab ORDER BY tname ASC;",
			),
		);
	}

	/**
	 * Connects to the host/database.
	 *
	 * @param string $conn_string Squiz Matrix system root.
	 *
	 * @return boolean true on success, false on failure
	 */
	public function connect($conn_string)
	{
		$SYSTEM_ROOT = $conn_string;

		if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
		    echo "You need to supply the path to the System Root as the first argument\n";
			exit(1);
		}

		require_once $SYSTEM_ROOT.'/fudge/dev/dev.inc';
		require_once $SYSTEM_ROOT.'/core/include/general.inc';
		require_once $SYSTEM_ROOT.'/core/lib/DAL/DAL.inc';
		require_once $SYSTEM_ROOT.'/core/lib/MatrixDAL/MatrixDAL.inc';
		require_once $SYSTEM_ROOT.'/data/private/conf/db.inc';

		$this->_dsn = $db_conf['db2'];
		$this->_db_type = $db_conf['db2']['type'];

		// Attempt to connect
		MatrixDAL::dbConnect($this->_dsn, $this->_db_type);
		MatrixDAL::changeDb($this->_db_type);

		// Matrix will throw a FATAL error if it can't connect, so if we got here
		// we're all good
		return true;
	}

	/**
	 * Get the name of the current database.
	 *
	 * @return string Name of the database.
	 */
	public function getDbName()
	{
		// Shorten the DB name if Oracle is using the full specifier
		$dsn = trim($this->_dsn['DSN']);
		if (preg_match("/SERVICE_NAME/i", $dsn)) {
			return preg_replace("/\A.*HOST\s*=\s*(.*?)\).*SERVICE_NAME\s*=\s*(.*?)\).*\z/i", '$2 on $1', $dsn);
		} else {
			return $dsn;
		}
	}

	/**
	 * Get a description of the database/backend type.
	 *
	 * @return string Name of the database system.
	 */
	public function getDbType()
	{
		return $this->_db_type;
	}

	/**
	 * Get the version of the database/backend type.
	 *
	 * @return string Version of the database system.
	 */
	public function getDbVersion()
	{
		return '';
	}

	/**
	 * Disconnect from the database/host.
	 *
	 * @return boolean true on success, false on failure
	 */
	public function disconnect()
	{
		return true;
	}

	/**
	 * Execute the specified SQL/commands on the database.
	 *
	 * @param string $sql The SQL/command to send to the database.
	 *
	 * @return mixed string or array of returned data, or false on failure
	 */
	public function execute($sql)
	{
		$output = false;

		// Check/execute macros
		foreach ($this->_macros[$this->_db_type] as $pattern => $replacement) {
			$c = 0;
			$sql = str_replace($pattern, $replacement, $sql, $c);
			if ($c > 0) {
				break;
			}
		}

		// Strip semicolon from end if its Oracle
		if ($this->_db_type == 'oci') {
		    $sql = mb_substr($sql, 0, mb_strlen($sql)-1);
		}

		// Check what kind of query it is
		$query_type = $this->_getQueryType($sql);
		switch ($query_type) {

			case "SELECT":
				$output = MatrixDAL::executeSqlAssoc($sql);
				break;

			case "UPDATE":
			case "INSERT":
				$rows_affected = MatrixDAL::executeSql($sql);
				$output = $query_type . " " . $rows_affected;
				break;

			case "BEGIN":
				/* There is no return bool code, but according to PHP docs an exception will
				   be thrown if the DB doesn't support transactions */
				MatrixDAL::beginTransaction();
				$output = $query_type;
				break;

			case "ROLLBACK":
				MatrixDAL::rollBack();
				$output = $query_type;
				break;

			case "COMMIT":
				MatrixDAL::commit();
				$output = $query_type;
				break;

			default:
				//echo "WARNING: Query type not recognised.\n";
				$output = MatrixDAL::executeSqlAssoc($sql);
				break;
		}

		return $output;
	}

	/**
	 * Get a list of the available tables on the current database. Used for
	 * autocomplete.
	 *
	 * @return array List of table names.
	 */
	public function getTableNames()
	{
		$sql = '';

		switch ($this->_db_type) {

			case 'pgsql':
				$sql = <<<EOF
					-- phpsqlc: tab-completion: table-names
					SELECT
					  c.relname as "Name"
					FROM pg_catalog.pg_class c
					     JOIN pg_catalog.pg_roles r ON r.oid = c.relowner
					     LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
					WHERE c.relkind IN ('r','')
					      AND n.nspname NOT IN ('pg_catalog', 'pg_toast')
					      AND pg_catalog.pg_table_is_visible(c.oid)
					ORDER BY 1;
EOF;
				break;

			case 'oci':
				// Cheeky UNION here to allow tab completion to work for both all-upper OR
				// all-lowercase table names (only for MatrixDAL/oci, so users can be lazy)
				$sql = "SELECT tname FROM tab UNION SELECT LOWER(tname) FROM tab";
				break;
		}

		// We only know queries for pgsql and oci
		if ($sql === '') {
			$names = array();

		} else {

			try {
				$names = MatrixDAL::executeSqlAssoc($sql, 0);
			}
			catch (Exception $e) {
				$names = array();
			}
		}

		return $names;
	}

	/**
	 * Get a list of the available columns on the specified table. Used for
	 * autocomplete.
	 *
	 * @param string $table Name of the table
	 *
	 * @return array List of column names
	 */
	public function getColumnNames($table)
	{
		$sql = '';

		switch ($this->_db_type) {

			case 'oci':
				// Cheeky UNION here to allow tab completion to work for both all-upper OR
				// all-lowercase table names (only for MatrixDAL/oci, so users can be lazy)
				$sql = "SELECT column_name FROM all_tab_columns WHERE table_name = " . mb_strtoupper(MatrixDAL::quote($table)) . " UNION " .
				       "SELECT LOWER(column_name) FROM all_tab_columns WHERE table_name = " . mb_strtoupper(MatrixDAL::quote($table));
				break;

			case 'pgsql':
				$sql = <<<EOF
					-- phpsqlc: tab-completion: column-names
					SELECT a.attname FROM pg_catalog.pg_attribute a
					WHERE a.attrelid IN (
					    SELECT c.oid
					    FROM pg_catalog.pg_class c
					         LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
					    WHERE c.relname = '$table' AND pg_catalog.pg_table_is_visible(c.oid)
					) AND a.attnum > 0 AND NOT a.attisdropped;
EOF;

		}

		// We only know queries for pgsql and oci
		if ($sql === '') {
			return array();
		}

		try {
		    $names = MatrixDAL::executeSqlAssoc($sql, 0);
		}
		catch (Exception $e) {
		    $names = array();
		}

		return $names;
	}

	/**
	 * Checks whether the specified command is a supported or valid macro.
	 *
	 * @param string $s Command
	 *
	 * @return boolean true if yes or false if not
	 */
	public function matchesMacro($s)
	{
		return array_key_exists(trim($s), $this->_macros[$this->_db_type]);
	}

	/**
	 * Returns the type of input query this is. Eg. SELECT, UPDATE, INSERT, etc.
	 *
	 * @param string $input_query The SQL string
	 *
	 * @return mixed A string containing type of query, in uppercase, or false
	 */
	private function _getQueryType($input_query)
	{
		$input_query = mb_strtoupper($input_query);

		if (mb_strpos($input_query, "SELECT") === 0) {
			$type = "SELECT";
		} elseif (mb_strpos($input_query, "UPDATE") === 0) {
			$type = "UPDATE";
		} elseif (mb_strpos($input_query, "INSERT INTO") === 0) {
			$type = "INSERT";
		} elseif (mb_strpos($input_query, "BEGIN") === 0) {
			$type = "BEGIN";
		} elseif (mb_strpos($input_query, "START TRANSACTION") === 0) {
			$type = "BEGIN";
		} elseif (mb_strpos($input_query, "ABORT") === 0) {
			$type = "ROLLBACK";
		} elseif (mb_strpos($input_query, "ROLLBACK") === 0) {
			$type = "ROLLBACK";
		} elseif (mb_strpos($input_query, "COMMIT") === 0) {
			$type = "COMMIT";
		} else {
			$type = false;
		}

		return $type;
	}
}

?>

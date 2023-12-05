extends Node

var _status : int = 0
var _stream: StreamPeerTCP = StreamPeerTCP.new()

func _ready() -> void:
	_status = _stream.get_status()
	connect_to_host("218.55.30.207", 55555)

func _process(delta: float) -> void:
	_stream.poll()
	var new_status: int = _stream.get_status()
	if new_status != _status:
		_status = new_status
		match _status:
			_stream.STATUS_NONE:
				print("Disconnected from host.")
			_stream.STATUS_CONNECTING:
				print("Connecting to host.")
			_stream.STATUS_CONNECTED:
				print("Connected to host.")
			_stream.STATUS_ERROR:
				print("Error with socket stream.")

	if _status == _stream.STATUS_CONNECTED:
		var available_bytes: int = _stream.get_available_bytes()
		if available_bytes > 0:
			print("available bytes: ", available_bytes)
			var data: Array = _stream.get_partial_data(available_bytes)
			# Check for read error.
			if data[0] != OK:
				print("Error getting data from stream: ", data[0])
			else:
				var text : String = ""
				for i in data[1]:
					text += char(i)
				print("data : ", text)
				get_node("CanvasLayer/Control/Label").text += text

func connect_to_host(host: String, port: int) -> void:
	print("Connecting to %s:%d" % [host, port])
	# Reset status so we can tell if it changes to error again.
	_status = _stream.STATUS_NONE
	if _stream.connect_to_host(host, port) != OK:
		print("Error connecting to host.")

func _on_button_pressed():
	var txt : String = get_node("CanvasLayer/Control/TextEdit").text
	get_node("CanvasLayer/Control/TextEdit").clear()
	var err : int = _stream.put_data(txt.to_utf8_buffer())
	
	if err != OK:
		print("failed to send data...")

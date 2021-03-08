C_FLAGS := -DDEBUG=1


chat_app: chat_app.c
	gcc $(C_FLAGS) -c -o $@ $< -Iinclude

server: server.c
	gcc $(C_FLAGS) -c -o $@ $< -Iinclude

clean:
	rm -f chat_app server

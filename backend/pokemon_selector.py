import requests
import random
import json
import urllib3
import ssl
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# Disable SSL warnings for environments with SSL issues
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

class PokemonSelector:
    def __init__(self):
        # Try HTTP first to avoid SSL issues
        self.base_url = "http://pokeapi.co/api/v2/"
        self.https_url = "https://pokeapi.co/api/v2/"
        self.stats = ["hp", "attack", "defense", "special-attack", "special-defense", "speed"]
        
        # Configure session with minimal SSL requirements
        self.session = requests.Session()
        
        # Disable SSL verification completely
        self.session.verify = False
        
        # Set basic headers
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
            'Accept': 'application/json',
            'Accept-Language': 'en-US,en;q=0.9',
            'Cache-Control': 'no-cache'
        })
    
    def get_pokemon_data(self, pokemon_name):
        """Fetch Pokemon data from PokeAPI with multiple URL strategies"""
        pokemon_name = pokemon_name.lower()
        
        # Strategy 1: Try HTTP (no SSL)
        try:
            url = f"{self.base_url}pokemon/{pokemon_name}"
            print(f"Trying HTTP: {url}")
            response = self.session.get(url, timeout=15)
            response.raise_for_status()
            print("✓ Connected successfully via HTTP")
            return response.json()
        except requests.exceptions.HTTPError as e:
            if e.response.status_code == 404:
                print(f"❌ Pokemon '{pokemon_name}' not found (404). Check spelling.")
                return None
            print(f"⚠ HTTP error {e.response.status_code}, trying HTTPS...")
        except Exception as e:
            print(f"⚠ HTTP failed: {e}")
            print("Trying HTTPS...")
        
        # Strategy 2: Try HTTPS with no SSL verification
        try:
            url = f"{self.https_url}pokemon/{pokemon_name}"
            print(f"Trying HTTPS (no SSL verify): {url}")
            response = requests.get(url, timeout=15, verify=False, headers={
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            })
            response.raise_for_status()
            print("✓ Connected successfully via HTTPS (no SSL)")
            return response.json()
        except requests.exceptions.HTTPError as e:
            if e.response.status_code == 404:
                print(f"❌ Pokemon '{pokemon_name}' not found (404). Check spelling.")
                return None
            print(f"❌ HTTPS error {e.response.status_code}")
        except Exception as e:
            print(f"❌ HTTPS failed: {e}")
        
        # Strategy 3: Try with urllib directly (lower level)
        try:
            import urllib.request
            import urllib.error
            import ssl
            
            # Create SSL context that ignores certificate errors
            ssl_context = ssl.create_default_context()
            ssl_context.check_hostname = False
            ssl_context.verify_mode = ssl.CERT_NONE
            
            url = f"{self.https_url}pokemon/{pokemon_name}"
            print(f"Trying urllib with custom SSL: {url}")
            
            req = urllib.request.Request(url, headers={
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            })
            
            with urllib.request.urlopen(req, context=ssl_context, timeout=15) as response:
                data = response.read()
                print("✓ Connected successfully via urllib")
                return json.loads(data.decode('utf-8'))
                
        except urllib.error.HTTPError as e:
            if e.code == 404:
                print(f"❌ Pokemon '{pokemon_name}' not found (404). Check spelling.")
                return None
            print(f"❌ urllib HTTPError {e.code}")
        except Exception as e:
            print(f"❌ urllib failed: {e}")
        
        print("❌ All connection methods failed")
        return None
    
    def get_pokemon_moves(self, pokemon_data):
        """Extract available moves from Pokemon data"""
        moves = []
        for move in pokemon_data['moves']:
            moves.append(move['move']['name'])
        return moves
    
    def generate_random_ivs(self):
        """Generate random IVs (0-31 for each stat)"""
        return {stat: random.randint(0, 31) for stat in self.stats}
    
    def generate_random_evs(self):
        """Generate random EVs (total max 510, max 252 per stat)"""
        evs = {stat: 0 for stat in self.stats}
        remaining_evs = 510
        
        # Distribute EVs randomly while respecting constraints
        for stat in self.stats[:-1]:  # Leave last stat for remainder
            if remaining_evs > 0:
                max_for_stat = min(252, remaining_evs)
                ev_value = random.randint(0, max_for_stat)
                evs[stat] = ev_value
                remaining_evs -= ev_value
        
        # Put remaining EVs in the last stat (if any)
        evs[self.stats[-1]] = min(252, remaining_evs)
        
        return evs
    
    def get_user_ivs(self):
        """Get IVs from user input"""
        print("\nEnter IVs (0-31) for each stat (or press Enter for random):")
        ivs = {}
        
        for stat in self.stats:
            while True:
                user_input = input(f"{stat.capitalize()} IV (0-31): ").strip()
                
                if user_input == "":
                    ivs[stat] = random.randint(0, 31)
                    print(f"Random {stat} IV: {ivs[stat]}")
                    break
                
                try:
                    iv_value = int(user_input)
                    if 0 <= iv_value <= 31:
                        ivs[stat] = iv_value
                        break
                    else:
                        print("IV must be between 0 and 31.")
                except ValueError:
                    print("Please enter a valid number.")
        
        return ivs
    
    def get_user_evs(self):
        """Get EVs from user input"""
        print("\nEnter EVs (0-252, total max 510) for each stat (or press Enter for random):")
        evs = {}
        total_evs = 0
        
        for stat in self.stats:
            while True:
                remaining = 510 - total_evs
                if remaining <= 0:
                    evs[stat] = 0
                    print(f"{stat.capitalize()} EV: 0 (no EVs remaining)")
                    break
                
                user_input = input(f"{stat.capitalize()} EV (0-{min(252, remaining)}): ").strip()
                
                if user_input == "":
                    max_ev = min(252, remaining)
                    evs[stat] = random.randint(0, max_ev)
                    print(f"Random {stat} EV: {evs[stat]}")
                    total_evs += evs[stat]
                    break
                
                try:
                    ev_value = int(user_input)
                    if 0 <= ev_value <= min(252, remaining):
                        evs[stat] = ev_value
                        total_evs += ev_value
                        break
                    else:
                        print(f"EV must be between 0 and {min(252, remaining)}.")
                except ValueError:
                    print("Please enter a valid number.")
        
        return evs
    
    def get_user_moves(self, available_moves):
        """Get moves from user input"""
        print(f"\nAvailable moves ({len(available_moves)} total):")
        print("First 20 moves:", ", ".join(available_moves[:20]))
        if len(available_moves) > 20:
            print("... and more")
        
        print("\nSelect 4 moves (or type 'random' for random selection):")
        moves = []
        
        for i in range(4):
            while True:
                user_input = input(f"Move {i+1}: ").strip().lower()
                
                if user_input == "random":
                    remaining_moves = [move for move in available_moves if move not in moves]
                    if remaining_moves:
                        selected_move = random.choice(remaining_moves)
                        moves.append(selected_move)
                        print(f"Random move selected: {selected_move}")
                        break
                    else:
                        print("No more moves available for random selection.")
                elif user_input in [move.lower() for move in available_moves]:
                    # Find the actual move name with correct capitalization
                    actual_move = next(move for move in available_moves if move.lower() == user_input)
                    if actual_move not in moves:
                        moves.append(actual_move)
                        break
                    else:
                        print("Move already selected. Choose a different move.")
                elif user_input == "":
                    print("Please enter a move name or 'random'.")
                else:
                    print("Move not available. Please choose from the available moves or type 'random'.")
        
        return moves
    
    def display_pokemon_summary(self, pokemon_data, ivs, evs, moves):
        """Display the final Pokemon summary"""
        print("\n" + "="*50)
        print("POKEMON SUMMARY")
        print("="*50)
        print(f"Name: {pokemon_data['name'].capitalize()}")
        print(f"Height: {pokemon_data['height']/10} m")
        print(f"Weight: {pokemon_data['weight']/10} kg")
        print(f"Types: {', '.join([t['type']['name'].capitalize() for t in pokemon_data['types']])}")
        
        print("\nBase Stats:")
        for stat in pokemon_data['stats']:
            print(f"  {stat['stat']['name'].capitalize()}: {stat['base_stat']}")
        
        print("\nIVs:")
        for stat, iv in ivs.items():
            print(f"  {stat.capitalize()}: {iv}")
        
        print("\nEVs:")
        for stat, ev in evs.items():
            print(f"  {stat.capitalize()}: {ev}")
        
        print("\nSelected Moves:")
        for i, move in enumerate(moves, 1):
            print(f"  {i}. {move.replace('-', ' ').title()}")
        
        print("="*50)
    
    def test_connection(self):
        """Test connection to PokeAPI"""
        print("Testing connection to PokeAPI...")
        
        # Test 1: Try HTTP first
        try:
            response = requests.get("http://pokeapi.co/api/v2/pokemon/1", timeout=10, headers={
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            })
            if response.status_code == 200:
                print("✓ PokeAPI is accessible via HTTP")
                return True
        except Exception as e:
            print(f"⚠ HTTP test failed: {e}")
        
        # Test 2: Try HTTPS without SSL verification
        try:
            response = requests.get("https://pokeapi.co/api/v2/pokemon/1", timeout=10, verify=False, headers={
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            })
            if response.status_code == 200:
                print("✓ PokeAPI is accessible via HTTPS (no SSL)")
                return True
        except Exception as e:
            print(f"⚠ HTTPS test failed: {e}")
        
        # Test 3: Try with urllib
        try:
            import urllib.request
            import ssl
            
            ssl_context = ssl.create_default_context()
            ssl_context.check_hostname = False
            ssl_context.verify_mode = ssl.CERT_NONE
            
            req = urllib.request.Request("https://pokeapi.co/api/v2/pokemon/1", headers={
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            })
            
            with urllib.request.urlopen(req, context=ssl_context, timeout=10) as response:
                if response.status == 200:
                    print("✓ PokeAPI is accessible via urllib")
                    return True
        except Exception as e:
            print(f"❌ urllib test failed: {e}")
        
        print("❌ Cannot connect to PokeAPI with any method")
        print("Please check your internet connection or network firewall settings.")
        return False
    
    def get_random_pokemon(self):
        """Get a random Pokemon from the first 1010 Pokemon (original generations)"""
        # Use Pokemon IDs 1-1010 to cover most generations but avoid newer forms
        random_id = random.randint(1, 1010)
        
        print(f"🎲 Getting random Pokemon #{random_id}...")
        
        # Try to get the Pokemon data using the ID
        pokemon_data = self.get_pokemon_data(str(random_id))
        if pokemon_data:
            print(f"✅ Random Pokemon: {pokemon_data['name'].capitalize()}!")
            return pokemon_data
        else:
            # If that ID doesn't work, try a few more times
            for _ in range(3):
                random_id = random.randint(1, 800)  # Fallback to more reliable range
                pokemon_data = self.get_pokemon_data(str(random_id))
                if pokemon_data:
                    print(f"✅ Random Pokemon: {pokemon_data['name'].capitalize()}!")
                    return pokemon_data
        
        # Final fallback to some known Pokemon
        fallback_pokemon = ['pikachu', 'charizard', 'blastoise', 'venusaur', 'alakazam', 'machamp']
        fallback_name = random.choice(fallback_pokemon)
        print(f"🔄 Using fallback Pokemon: {fallback_name}")
        return self.get_pokemon_data(fallback_name)
    
    def run(self):
        """Main application loop"""
        print("Welcome to the Pokemon Selector!")
        print("This app fetches Pokemon data from PokeAPI.")
        
        # Test connection first
        if not self.test_connection():
            print("\n❌ Unable to connect to PokeAPI. Please check your internet connection and try again.")
            return
        
        print("Type 'quit' to exit the program.")
        
        while True:
            print("\n" + "-"*40)
            pokemon_name = input("Enter a Pokemon name: ").strip()
            
            if pokemon_name.lower() == 'quit':
                print("Thanks for using Pokemon Selector!")
                break
            
            if not pokemon_name:
                print("Please enter a Pokemon name.")
                continue
            
            # Fetch Pokemon data
            print(f"Fetching data for {pokemon_name}...")
            pokemon_data = self.get_pokemon_data(pokemon_name)
            
            if not pokemon_data:
                print("Please try again with a different Pokemon name.")
                continue
            
            print(f"✓ Found {pokemon_data['name'].capitalize()}!")
            
            # Get available moves
            available_moves = self.get_pokemon_moves(pokemon_data)
            
            # Get IVs
            print("\n--- IV Selection ---")
            choice = input("Enter IVs manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                ivs = self.generate_random_ivs()
                print("Random IVs generated!")
            else:
                ivs = self.get_user_ivs()
            
            # Get EVs
            print("\n--- EV Selection ---")
            choice = input("Enter EVs manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                evs = self.generate_random_evs()
                print("Random EVs generated!")
            else:
                evs = self.get_user_evs()
            
            # Get Moves
            print("\n--- Move Selection ---")
            choice = input("Select moves manually or use random? (manual/random): ").strip().lower()
            if choice == "random":
                moves = random.sample(available_moves, min(4, len(available_moves)))
                print("Random moves selected!")
            else:
                moves = self.get_user_moves(available_moves)
            
            # Display summary
            self.display_pokemon_summary(pokemon_data, ivs, evs, moves)
            
            # Ask if user wants to continue
            continue_choice = input("\nWould you like to select another Pokemon? (y/n): ").strip().lower()
            if continue_choice != 'y':
                print("Thanks for using Pokemon Selector!")
                break

if __name__ == "__main__":
    selector = PokemonSelector()
    selector.run()
